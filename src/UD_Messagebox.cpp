#include <UD_MessageBox.h>
#include <UD_Macros.h>
#include <UD_Config.h>
#include <UD_Utility.h>

SINGLETONBODY(UD::MessageBoxCalibration)
SINGLETONBODY(UD::MessageboxManager)

void UD::MessageboxManager::ShowMessageBox(const std::string& bodyText, const std::vector<std::string>& buttonTextValues, std::function<void(uint32_t)> callback, bool useHtml)
{
    auto* factoryManager = RE::MessageDataFactoryManager::GetSingleton();
    auto* uiStringHolder = RE::InterfaceStrings::GetSingleton();
    auto* factory = factoryManager->GetCreator<RE::MessageBoxData>(uiStringHolder->messageBoxData);  // "MessageBoxData" <--- can we just use this string?
    auto* messagebox = factory->Create();
    RE::BSTSmartPointer<RE::IMessageBoxCallback> messageCallback = RE::make_smart<MessageBoxResultCallback>(callback);
    messagebox->callback = messageCallback;
    messagebox->bodyText = bodyText;
    for (auto text : buttonTextValues) messagebox->buttonText.push_back(text.c_str());

    messagebox->unk4D = useHtml; // yes, this actually enables html

    MessageBoxCalibration::GetSingleton()->Inject(messagebox);

    messagebox->QueueMessage();
}

void UD::MessageboxManager::Setup()
{
    DEBUG("MessageBoxCalibration address => 0x{:016X}",(uintptr_t)MessageBoxCalibration::GetSingleton())
}

int32_t UD::MessageboxManager::ShowArrayNonBlocking(const std::string& bodyText, std::vector<std::string>& buttonTexts, bool useHtml)
{
    std::erase_if(buttonTexts, [](const std::string& text) { return text.empty(); });
    auto messageBoxId = GetNewMessageBoxId();
    _messageBoxButtons.emplace(messageBoxId, buttonTexts);
    ShowMessageBox(bodyText, buttonTexts, [messageBoxId,this](uint32_t result) 
    {
        if (messageBoxId) _messageBoxResults.insert_or_assign(messageBoxId, result);
    },useHtml);
    return messageBoxId;
}

int32_t UD::MessageboxManager::ShowArrayNonBlockingTemplate(RE::BGSMessage* Message, std::string bodyText, std::vector<float>& values, std::vector<std::string>& buttonTexts, bool useHtml)
{
    std::erase_if(buttonTexts, [](const std::string& text) { return text.empty(); });
    auto messageBoxId = GetNewMessageBoxId();
    _messageBoxButtons.emplace(messageBoxId, buttonTexts);

    std::vector<int32_t> _buttonresparse;
    if (buttonTexts.empty())
    {
        
        int32_t loc_button = 0;
        for (auto&& it : Message->menuButtons)
        {
            if (it->conditions.IsTrue(RE::PlayerCharacter::GetSingleton(),RE::PlayerCharacter::GetSingleton()))
            {
                buttonTexts.push_back(it->text.c_str());
                _buttonresparse.push_back(loc_button);
            }
            loc_button++;
        }
    }
    _messageBoxButtonsResParse.emplace(messageBoxId,_buttonresparse);

    std::string loc_text = bodyText;
    if (loc_text.empty())
    {
        loc_text = Message->fullName;
    }

    ShowMessageBox(loc_text, buttonTexts, [messageBoxId,this](uint32_t result) 
    {
        if (messageBoxId) _messageBoxResults.insert_or_assign(messageBoxId, result);
    },useHtml);
    return messageBoxId;
}

int32_t UD::MessageboxManager::ShowNonBlocking(const std::string& bodyText, const std::string& button1, const std::string& button2, const std::string& button3, const std::string& button4, const std::string& button5, const std::string& button6, const std::string& button7, const std::string& button8, const std::string& button9, const std::string& button10, bool useHtml)
{
    std::vector<std::string> buttonTexts;
    if (!button1.empty()) buttonTexts.push_back(button1);
    if (!button2.empty()) buttonTexts.push_back(button2);
    if (!button3.empty()) buttonTexts.push_back(button3);
    if (!button4.empty()) buttonTexts.push_back(button4);
    if (!button5.empty()) buttonTexts.push_back(button5);
    if (!button6.empty()) buttonTexts.push_back(button6);
    if (!button7.empty()) buttonTexts.push_back(button7);
    if (!button8.empty()) buttonTexts.push_back(button8);
    if (!button9.empty()) buttonTexts.push_back(button9);
    if (!button10.empty()) buttonTexts.push_back(button10);
    return ShowArrayNonBlocking(bodyText, buttonTexts,useHtml);
}

void UD::MessageboxManager::Delete(int32_t messageBoxId)
{
    if (_messageBoxResults.contains(messageBoxId)) _messageBoxResults.erase(messageBoxId);
    if (_messageBoxButtons.contains(messageBoxId)) _messageBoxButtons.erase(messageBoxId);
    if (_messageBoxButtonsResParse.contains(messageBoxId)) _messageBoxButtonsResParse.erase(messageBoxId);
}

std::string UD::MessageboxManager::GetResultText(int32_t messageBoxId, bool deleteResultOnAccess)
{
    std::string resultString;
    if (_messageBoxResults.contains(messageBoxId) && _messageBoxButtons.contains(messageBoxId)) 
    {
        auto index = _messageBoxResults.at(messageBoxId);
        auto buttons = _messageBoxButtons.at(messageBoxId);
        if (buttons.size() > index) resultString = buttons.at(index);
        if (deleteResultOnAccess) Delete(messageBoxId);
    }
    return resultString;
}

int32_t UD::MessageboxManager::GetResultIndex(int32_t messageBoxId, bool deleteResultOnAccess)
{
    auto index = -1;
    std::vector<int32_t> loc_resparse;
    if (_messageBoxResults.contains(messageBoxId)) 
    {
        index = _messageBoxResults.at(messageBoxId);
        if (_messageBoxButtonsResParse.contains(messageBoxId))
        {
            loc_resparse = _messageBoxButtonsResParse.at(messageBoxId);
        }
        
        if (deleteResultOnAccess) Delete(messageBoxId);
    }
    return (loc_resparse.size() > index) ? loc_resparse[index] : index;
}

bool UD::MessageboxManager::IsMessageResultAvailable(int32_t messageBoxId)
{
    return _messageBoxResults.contains(messageBoxId);
}

void UD::MessageBoxCalibration::Inject(RE::MessageBoxData* aData)
{
    if (inject & 0x01U) aData->unk38 = unk38; else unk38 = aData->unk38;
    if (inject & 0x02U) aData->unk3C = unk3C; else unk3C = aData->unk3C;
    if (inject & 0x04U) aData->unk48 = unk48; else unk48 = aData->unk48;
    if (inject & 0x08U) aData->unk4C = unk4C; else unk4C = aData->unk4C;
    if (inject & 0x10U) aData->unk4D = unk4D; else unk4D = aData->unk4D;
    if (inject & 0x20U) aData->unk4E = unk4E; else unk4E = aData->unk4E;
    if (inject & 0x40U) aData->unk4F = unk4F; else unk4F = aData->unk4F;
}
