#include "air.h"

int load_config(std::map<std::string, int> &keys)
{

    Json::Reader reader;
    Json::Value cfg;

    std::ifstream file("config.json");

    if (!reader.parse(file, cfg))
    {
        std::cout << reader.getFormattedErrorMessages();
        exit(1);
    }

    int i = 1; // LEAVE INDEX 0 EMPTY FOR KEY NOT FOUND DEFAULT
    auto vectors = cfg["vectors"];

    for (int v = 0; v < vectors.size(); v++)
    {
        auto vector = vectors[v];
        if (vector["enabled"].asBool())
        {
            auto ks = vector["keys"];
            for (int k = 0; k < ks.size(); k++)
            {
                if (!keys.insert(std::pair<std::string, int>(ks[k].asString(), i)).second)
                {
                    printf("DUPLICATE KEYS FOUND IN CONFIG FILE.\n");
                    std::exit(-1);
                }
                i++;
            }
        }
    }

    return 0;
}