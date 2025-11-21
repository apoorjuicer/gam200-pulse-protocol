// -----------------------------------------------------------------------------
// Gizmo.cpp  -  World transform gizmo (translate/scale/rotate) for editor
// -----------------------------------------------------------------------------

#include "pch/pch_temp.h"

// Core editor / engine headers
#include "Editor.h"   // for GameViewport, GizmoState, selectedEntity, INVALID_ENTITY
#include "Gizmo.h"
#include "../CoreEngine/ECS/Coordinator.h"
#include "../CoreEngine/ECS/Types.h"
#include "../Graphics/Transform.h"
#include "../Graphics/glapp.h"


// If you have a dedicated gizmo type, include it here
// #include "GizmoState.h"

namespace MyEditor
{
    // -------------------------------------------------------------------------
    // Internal state & constants
    // -------------------------------------------------------------------------

    // World logical size the editor gizmo assumes (match your camera setup)
    static constexpr float kWorldW = 1920.0f;
    static constexpr float kWorldH = 1080.0f;

    // Main gizmo state (mode, dragging flags, etc.)
    static GizmoState g_gizmo; // adjust type/name to your actual gizmo struct

    // -------------------------------------------------------------------------
    // Small helpers (local to this file)
    // -------------------------------------------------------------------------

    // Convert between your Vector2 and glm::vec2 (adjust if you use something else)
    static inline glm::vec2 ToGlm(const Vector2& v)
    {
        return { v.x, v.y };
    }

    static inline Vector2 ToV2(const glm::vec2& v)
    {
        return { v.x, v.y };
    }

    // Compute the rectangle where the game is drawn inside the editor window
    // (letterboxing / pillarboxing). This should match your GLApp / GraphicsManager.
    static void ComputeGameDrawRect(const GameViewport& vp, float& drawX, float& drawY,
        float& drawW, float& drawH, float& scale)
    {
        const float sx = vp.w / kWorldW;
        const float sy = vp.h / kWorldH;
        scale = (sx < sy) ? sx : sy;           // fit inside
        drawW = kWorldW * scale;
        drawH = kWorldH * scale;
        drawX = vp.x + (vp.w - drawW) * 0.5f;  // pillar/letter box offsets
        drawY = vp.y + (vp.h - drawH) * 0.5f;
    }

    // -------------------------------------------------------------------------
    // World <-> Screen conversion
    // -------------------------------------------------------------------------

       glm::vec2 WorldToScreen(const Vector2& wpos, const GameViewport& vp)
    {
        // world -> NDC
        const float sx = 2.0f / kWorldW, sy = 2.0f / kWorldH;
        const glm::mat3 N(sx, 0, 0, 0, sy, 0, 0, 0, 1);
        const glm::mat3 Cam = GLApp::BuildPanZoomNDC(GLApp::gCamera);
        const glm::vec3 ndc = Cam * (N * glm::vec3(wpos.x, wpos.y, 1.0f)); // -1..1

        // NDC -> framebuffer px inside the *drawn* game rect (letterbox-aware)
        float drawX, drawY, drawW, drawH, scale;
        ComputeGameDrawRect(vp, drawX, drawY, drawW, drawH, scale);

        const float x = drawX + (ndc.x * 0.5f + 0.5f) * drawW;
        const float y = drawY + (1.0f - (ndc.y * 0.5f + 0.5f)) * drawH;
        return { x, y };
    }


    Vector2 ScreenToWorld(glm::vec2 screen, const GameViewport& vp)
    {
        // framebuffer px -> NDC using the *drawn* game rect
        float drawX, drawY, drawW, drawH, scale;
        ComputeGameDrawRect(vp, drawX, drawY, drawW, drawH, scale);

        const float ndcX = ((screen.x - drawX) / drawW) * 2.0f - 1.0f;
        const float ndcY = 1.0f - ((screen.y - drawY) / drawH) * 2.0f;
        const glm::vec3 ndc(ndcX, ndcY, 1.0f);

        // invert camera + normalization (as you already do)
        const float sx = 2.0f / kWorldW, sy = 2.0f / kWorldH;
        const glm::mat3 N(sx, 0, 0, 0, sy, 0, 0, 0, 1);
        const glm::mat3 Cam = GLApp::BuildPanZoomNDC(GLApp::gCamera);
        const glm::mat3 invAll = glm::inverse(Cam * N);

        const glm::vec3 w = invAll * ndc;
        return Vector2{ w.x, w.y };
    }

    // -------------------------------------------------------------------------
    // Public entry point
    // -------------------------------------------------------------------------
    void Gizmo_DrawAndHandle(const GameViewport& vp)
    {
        auto* coord = Coordinator::GetInstance();
        if (!coord) return;

        ImGuiIO& io = ImGui::GetIO();

        // ImGui UI px  <->  Framebuffer px (handles DPI / HiDPI)
        const float sx_ui = io.DisplayFramebufferScale.x > 0.f ? io.DisplayFramebufferScale.x : 1.f;
        const float sy_ui = io.DisplayFramebufferScale.y > 0.f ? io.DisplayFramebufferScale.y : 1.f;

        // --- Letterbox-aware draw rectangle (in framebuffer px) ---
        float drawX, drawY, drawW, drawH, scale;
        ComputeGameDrawRect(vp, drawX, drawY, drawW, drawH, scale);

        // Mouse in framebuffer pixels
        const glm::vec2 mouseFB(io.MousePos.x * sx_ui, io.MousePos.y * sy_ui);

        // FB (draw rect) -> NDC
        const float ndcX = ((mouseFB.x - drawX) / drawW) * 2.0f - 1.0f;
        const float ndcY = 1.0f - ((mouseFB.y - drawY) / drawH) * 2.0f;
        const glm::vec3 mouseNDC(ndcX, ndcY, 1.0f);

        // Camera pan/zoom used by the renderer
        const glm::mat3 PanZoom = GLApp::BuildPanZoomNDC(GLApp::gCamera);

        // Normalization matrix used by renderer (world −1..1)
        const float nsx = 2.0f / kWorldW, nsy = 2.0f / kWorldH;
        const glm::mat3 N(nsx, 0, 0, 0, nsy, 0, 0, 0, 1);
        const glm::mat3 invAll = glm::inverse(PanZoom * N);

        auto ScreenToWorld_DrawRect = [&](glm::vec2 fb)->Vector2 {
            const float x = ((fb.x - drawX) / drawW) * 2.0f - 1.0f;
            const float y = 1.0f - ((fb.y - drawY) / drawH) * 2.0f;
            glm::vec3 w = invAll * glm::vec3(x, y, 1.0f);
            return Vector2{ w.x, w.y };
            };

        // Hover/pick using the same pipeline as rendering
        Entity hovered = INVALID_ENTITY;

        // Clip gizmo to the drawn game rect (convert FB px -> ImGui UI px)
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        dl->PushClipRect(ImVec2(drawX / sx_ui, drawY / sy_ui),
            ImVec2((drawX + drawW) / sx_ui, (drawY + drawH) / sy_ui),
            true);

        const bool inGameRect =
            (mouseFB.x >= drawX && mouseFB.x <= drawX + drawW &&
                mouseFB.y >= drawY && mouseFB.y <= drawY + drawH);

        for (Entity e : coord->GetAllEntities())
        {
            if (!coord->HasComponent<Framework::Transform>(e) ||
                !coord->HasComponent<Framework::Renderable>(e)) continue;

            auto& tr = coord->GetComponent<Framework::Transform>(e);
            auto& re = coord->GetComponent<Framework::Renderable>(e);
            if (!tr.isVisible) continue;

            // Model->NDC (same as draw path)
            glm::mat3 M = glm::make_mat3(tr.mdl_to_ndc_xform.Begin());
            glm::mat3 modelToNDC = PanZoom * M;
            glm::mat3 invModelToNDC = glm::inverse(modelToNDC);

            // Background uses [-1,1], sprites use [-0.5,0.5]
            glm::vec2 qmin, qmax;
            if (re.mdl_ref == 0) { qmin = { -1.f, -1.f }; qmax = { 1.f,  1.f }; }
            else { qmin = { -0.5f,-0.5f }; qmax = { 0.5f, 0.5f }; }

            // Hit test
            if (inGameRect) {
                glm::vec3 m = invModelToNDC * mouseNDC;
                if (m.x >= qmin.x && m.x <= qmax.x && m.y >= qmin.y && m.y <= qmax.y)
                    hovered = e;
            }

            // Draw bounds for the selected entity
            if (e == selectedEntity)
            {
                auto toUI = [&](glm::vec2 modelP)->ImVec2 {
                    glm::vec3 n = modelToNDC * glm::vec3(modelP, 1.0f);       // model -> NDC
                    const float x = drawX + (n.x * 0.5f + 0.5f) * drawW;       // NDC -> FB (draw rect)
                    const float y = drawY + (1.0f - (n.y * 0.5f + 0.5f)) * drawH;
                    return ImVec2(x / sx_ui, y / sy_ui);                       // FB -> UI
                    };

                ImVec2 a = toUI({ qmin.x, qmin.y });
                ImVec2 b = toUI({ qmax.x, qmin.y });
                ImVec2 c = toUI({ qmax.x, qmax.y });
                ImVec2 d = toUI({ qmin.x, qmax.y });

                dl->AddLine(a, b, IM_COL32(255, 220, 80, 180), 2.0f);
                dl->AddLine(b, c, IM_COL32(255, 220, 80, 180), 2.0f);
                dl->AddLine(c, d, IM_COL32(255, 220, 80, 180), 2.0f);
                dl->AddLine(d, a, IM_COL32(255, 220, 80, 180), 2.0f);
            }
        }

        // Drag logic (world-space; now using draw-rect aware conversion)
        if (inGameRect && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            if (hovered != INVALID_ENTITY) {
                selectedEntity = hovered;
                auto& tr = coord->GetComponent<Framework::Transform>(selectedEntity);
                Vector2 mouseWorld = ScreenToWorld_DrawRect(mouseFB);
                g_gizmo.dragging = true;
                g_gizmo.active = selectedEntity;
                g_gizmo.dragStartWorld = mouseWorld;
                g_gizmo.entityStartPos = tr.Pos;
            }
            else {
                selectedEntity = INVALID_ENTITY;
            }
        }

        if (g_gizmo.dragging && g_gizmo.active != INVALID_ENTITY && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            if (coord->HasComponent<Framework::Transform>(g_gizmo.active))
            {
                auto& tr = coord->GetComponent<Framework::Transform>(g_gizmo.active);
                Vector2 mouseWorld = ScreenToWorld_DrawRect(mouseFB);
                Vector2 d{ mouseWorld.x - g_gizmo.dragStartWorld.x,
                           mouseWorld.y - g_gizmo.dragStartWorld.y };
                tr.Pos.x = g_gizmo.entityStartPos.x + d.x;
                tr.Pos.y = g_gizmo.entityStartPos.y + d.y;
            }
        }

        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            g_gizmo.dragging = false;
            g_gizmo.active = INVALID_ENTITY;
        }

        dl->PopClipRect();
    }

   

} // namespace MyEditor
