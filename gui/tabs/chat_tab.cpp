#include "pch-il2cpp.h"
#include "chat_tab.h"
#include "game.h"
#include "gui-helpers.hpp"
#include "utility.h"
#include "state.hpp"

namespace ChatTab {

    void Render() {
        ImGui::SameLine(100 * State.dpiScale);
        ImGui::BeginChild("###Chat", ImVec2(500 * State.dpiScale, 0), true, ImGuiWindowFlags_NoBackground);

        bool msgAllowed = IsChatValid(State.chatMessage);
        if (!msgAllowed) {
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.5f, 0.f, 0.f, State.MenuThemeColor.w));
            if (InputStringMultiline("\n\n\n\n\nChat Message", &State.chatMessage)) State.Save();
            ImGui::PopStyleColor();
        }
        else if (InputStringMultiline("\n\n\n\n\nChat Message", &State.chatMessage)) State.Save();

        if ((IsInGame() || IsInLobby()) && State.ChatCooldown >= 3.f && IsChatValid(State.chatMessage)) {
            ImGui::SameLine();
            if (AnimatedButton("Send")) {
                auto player = (!State.SafeMode && State.playerToChatAs.has_value()) ?
                    State.playerToChatAs.validate().get_PlayerControl() : *Game::pLocalPlayer;
                if (IsInGame()) State.rpcQueue.push(new RpcSendChat(player, State.chatMessage));
                else if (IsInLobby()) State.lobbyRpcQueue.push(new RpcSendChat(player, State.chatMessage));
                State.MessageSent = true;
            }
        }

        if ((IsInGame() || IsInLobby()) && State.ReadAndSendTerraChat) ImGui::SameLine();
        if (State.ReadAndSendTerraChat && (IsInGame() || IsInLobby()) && AnimatedButton("Send TerraChat")) {
            auto player = (!State.SafeMode && State.playerToChatAs.has_value()) ?
                State.playerToChatAs.validate().get_PlayerControl() : *Game::pLocalPlayer;
            if (IsInGame()) {
                State.rpcQueue.push(new RpcForceTerraChat(PlayerSelection(player), State.chatMessage, true));
            }
            else if (IsInLobby()) {
                State.lobbyRpcQueue.push(new RpcForceTerraChat(PlayerSelection(player), State.chatMessage, true));
            }
        }

        if (ToggleButton("Spam", &State.ChatSpam)) {
            if (State.BrainrotEveryone) State.BrainrotEveryone = false;
            if (State.RizzUpEveryone) State.RizzUpEveryone = false;
            State.Save();
        }
        if (((IsHost() && IsInGame()) || !State.SafeMode) && State.ChatSpamMode) ImGui::SameLine();
        if ((IsHost() || !State.SafeMode) && State.ChatSpamMode && ToggleButton("Spam by Everyone", &State.ChatSpamEveryone)) {
            State.Save();
        }
        if (IsHost() || !State.SafeMode) {
            if (CustomListBoxInt("Chat Spam Mode", &State.ChatSpamMode,
                { State.SafeMode ? "With Message (Self-Spam ONLY)" : "With Message", "Blank Chat", State.SafeMode ? "Self Message + Blank Chat" : "Message + Blank Chat" })) State.Save();
        }

        if (std::find(State.ChatPresets.begin(), State.ChatPresets.end(), State.chatMessage) == State.ChatPresets.end() && AnimatedButton("Add Message as Preset")) {
            State.ChatPresets.push_back(State.chatMessage);
            State.Save();
        }
        if (!(IsHost() || !State.SafeMode) && State.chatMessage.size() > 120) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Message will be detected by anticheat.");
        }
        if (!State.ChatPresets.empty()) {
            static int selectedPresetIndex = 0;
            selectedPresetIndex = std::clamp(selectedPresetIndex, 0, (int)State.ChatPresets.size() - 1);
            std::vector<const char*> presetVector(State.ChatPresets.size(), nullptr);
            for (size_t i = 0; i < State.ChatPresets.size(); i++) {
                presetVector[i] = State.ChatPresets[i].c_str();
            }
            CustomListBoxInt("Message to Send/Remove", &selectedPresetIndex, presetVector);
            auto msg = State.ChatPresets[selectedPresetIndex];
            if (AnimatedButton("Set as Chat Message")) {
                State.chatMessage = msg;
            }
            ImGui::SameLine();
            if (AnimatedButton("Remove"))
                State.ChatPresets.erase(State.ChatPresets.begin() + selectedPresetIndex);
        }

        ImGui::EndChild();
    }

}
