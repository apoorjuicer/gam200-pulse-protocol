#include <pch/pch.h>

    bool SaveEntity(Entity e, Coordinator* g, std::string const& path) {
        using namespace rapidjson;
        Document d;
        d.SetObject();
        auto& alloc = d.GetAllocator();

        // NPC
        if (g->HasComponent<NPC>(e)) {
            //Value npcVal;
            Value npcVal(kObjectType);
            g->GetComponent<NPC>(e).Serialize(npcVal, alloc);
            d.AddMember("NPC", npcVal, alloc);
        }

        //// AABB
        //if (g->HasComponent<AABB>(e)) {
        //    Value aabbVal;
        //    g->GetComponent<AABB>(e).Serialize(aabbVal, alloc);
        //    d.AddMember("AABB", aabbVal, alloc);
        //}

        StringBuffer buffer;
        PrettyWriter<StringBuffer> writer(buffer);
        d.Accept(writer);
        namespace fs = std::filesystem;
        //fs::create_directories("../../MyEngine/JSON");
        fs::create_directories("../JSON");


        std::ofstream ofs(path);
        if (!ofs.is_open()) {
            std::cerr << "Error: Cannot open file for writing: " << path << "\n";
            return false;
        }
        ofs << buffer.GetString();
    

        if (!ofs.good()) {
            std::cerr << "Error: Failed to write JSON to file: " << path << "\n";
            return false;
        }

        return true;
    }

    bool LoadEntity(Entity e, Coordinator* g, std::string const& path) {
        using namespace rapidjson;
        std::ifstream ifs(path);
        if (!ifs.is_open()) return false;

        IStreamWrapper isw(ifs);
        Document d;
        d.ParseStream(isw);

        if (!d.IsObject()) return false;

        if (d.HasMember("NPC")) {
            auto& npc = g->GetComponent<NPC>(e);
            npc.Deserialize(d["NPC"]);
        }
    /*    if (d.HasMember("AABB")) {
            auto& aabb = g->GetComponent<AABB>(e);
            aabb.Deserialize(d["AABB"]);
        }*/

        return true;
    }

