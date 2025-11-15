#pragma once

namespace MyEditor
{
    std::string OpenJsonFileDialog();

    // Play-mode baseline helpers
    void CaptureEditorBaseline();
    void RestoreEditorBaseline();

    // UI draw function
    void DrawTopBarPanel(float windowWidth, float top_y, float top_h);
}
