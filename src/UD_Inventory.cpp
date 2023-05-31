#include <UD_Inventory.h>
namespace UD 
{
    RE::TESDataHandler* InventoryHandler::datahandler = nullptr;
    std::pair<std::vector<RE::BGSKeyword*>,bool> InventoryHandler::invfilter = std::pair<std::vector<RE::BGSKeyword*>,bool>(std::vector<RE::BGSKeyword*>(),true);
    std::pair<std::vector<RE::BGSKeyword*>,bool> InventoryHandler::renfilter = std::pair<std::vector<RE::BGSKeyword*>,bool>(std::vector<RE::BGSKeyword*>(),true);
}
