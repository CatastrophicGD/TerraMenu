#include "pch-il2cpp.h"
#include "esp.hpp"
#include "DirectX.h"
#include "utility.h"
#include "game.h"
#include "gui-helpers.hpp"

drawing_t* Esp::s_Instance = new drawing_t();
ImGuiWindow* CurrentWindow = nullptr;

static void RenderText(std::string_view text, const ImVec2& pos, const ImVec4& color, const bool outlined = true, const bool centered = true)
{
        if (text.empty() || State.PanicMode) return;
        ImVec2 ImScreen = pos;
        if (centered)
        {
                auto size = ImGui::CalcTextSize(text.data(), text.data() + text.length());
                ImScreen.x -= size.x * 0.5f;
                ImScreen.y -= size.y;
        }

        if (outlined)
        {
                CurrentWindow->DrawList->AddText(nullptr, 0.f,
                        ImScreen + 0.5f * State.dpiScale,
                        ImGui::GetColorU32(IM_COL32_BLACK), text.data(), text.data() + text.length());
        }

        CurrentWindow->DrawList->AddText(nullptr, 0.f, ImScreen, ImGui::GetColorU32(color), text.data(), text.data() + text.length());
}

static void RenderLine(const ImVec2& start, const ImVec2& end, const ImVec4& color, bool shadow = false) noexcept
{
        if (State.PanicMode) return;

        if (shadow)
        {
                CurrentWindow->DrawList->AddLine(
                        start + 0.5f * State.dpiScale,
                        end + 0.5f * State.dpiScale,
                        ImGui::GetColorU32(color) & IM_COL32_A_MASK, 1.0f * State.dpiScale);
        }

        CurrentWindow->DrawList->AddLine(start, end, ImGui::GetColorU32(color), 1.0f * State.dpiScale);
}

static void RenderBox(const ImVec2& top, const ImVec2& bottom, const float height, const float width, const ImVec4& color, const bool wantsShadow = true)
{
        if (State.PanicMode) return;

        const ImVec2 points[] = {
                bottom, { bottom.x, ((float)(int)(bottom.y * 0.75f + top.y * 0.25f)) },
                { bottom.x - 0.5f * State.dpiScale, bottom.y }, { ((float)(int)(bottom.x * 0.75f + top.x * 0.25f)), bottom.y },

                { top.x + 0.5f * State.dpiScale, bottom.y }, { ((float)(int)(top.x * 0.75f + bottom.x * 0.25f)), bottom.y },
                { top.x, bottom.y }, { top.x, ((float)(int)(bottom.y * 0.75f + top.y * 0.25f)) },

                { bottom.x, top.y }, { bottom.x, ((float)(int)(top.y * 0.75f + bottom.y * 0.25f)) },
                { bottom.x - 0.5f * State.dpiScale, top.y }, { ((float)(int)(bottom.x * 0.75f + top.x * 0.25f)), top.y },

                top, { ((float)(int)(top.x * 0.75f + bottom.x * 0.25f)), top.y },
                { top.x, top.y + 0.5f * State.dpiScale }, { top.x, ((float)(int)(top.y * 0.75f + bottom.y * 0.25f)) }
        };

        if (wantsShadow) {
                const ImVec4& shadowColor = ImGui::ColorConvertU32ToFloat4(ImGui::GetColorU32(color) & IM_COL32_A_MASK);
                for (size_t i = 0; i < std::size(points); i += 2) {
                        RenderLine(points[i] + 0.5f * State.dpiScale, points[i + 1] + 0.5f * State.dpiScale, shadowColor, false);
                }
        }
        for (size_t i = 0; i < std::size(points); i += 2) {
                RenderLine(points[i], points[i + 1], color, false);
        }
}

// Ported from HyperMenu TracersHandler.cs: GetDistanceBasedColor
// Red = close (<=2 units), Yellow = medium, Green = far (>=20 units)
static ImVec4 GetDistanceBasedColor(float distance)
{
        constexpr float maxDistance = 20.0f;
        constexpr float minDistance = 2.0f;

        float normalized = (distance - minDistance) / (maxDistance - minDistance);
        if (normalized < 0.0f) normalized = 0.0f;
        if (normalized > 1.0f) normalized = 1.0f;

        const ImVec4 red    = { 1.0f, 0.0f, 0.0f, 1.0f };
        const ImVec4 yellow = { 1.0f, 1.0f, 0.0f, 1.0f };
        const ImVec4 green  = { 0.0f, 1.0f, 0.0f, 1.0f };

        if (normalized < 0.5f) {
                float t = normalized * 2.0f;
                return { red.x + (yellow.x - red.x) * t,
                         red.y + (yellow.y - red.y) * t,
                         red.z + (yellow.z - red.z) * t, 1.0f };
        } else {
                float t = (normalized - 0.5f) * 2.0f;
                return { yellow.x + (green.x - yellow.x) * t,
                         yellow.y + (green.y - yellow.y) * t,
                         yellow.z + (green.z - yellow.z) * t, 1.0f };
        }
}

// Compute the tracer color for a player entry using HyperMenu's logic:
//   - roleColorTracers:     player's actual role color
//   - distanceBasedTracers: red(close) -> yellow -> green(far)
//   - colorBasedTracers:    player's cosmetic/outfit color
//   - default:              role/team color (it.Color, already computed by the hook)
static ImVec4 GetTracerColor(const EspPlayerData& it)
{
        if (State.ShowEsp_RoleColorTracers)
                return it.RoleColor;
        if (State.ShowEsp_DistanceBasedTracers)
                return GetDistanceBasedColor(it.Distance);
        if (State.ShowEsp_ColorBasedTracers)
                return it.CosmeticColor;
        // Default: ghosts draw white, others use the pre-computed color (role or cosmetic)
        if (it.IsDead)
                return { 1.0f, 1.0f, 1.0f, 1.0f };
        return it.Color;
}

bool renderPlayerEsp = false;

void Esp::Render()
{
        if (State.PanicMode) return;

        CurrentWindow = ImGui::GetCurrentWindow();

        drawing_t& instance = Esp::GetDrawing();

        // Lock our mutex when we render (this will unlock when it goes out of scope)
        synchronized(instance.m_DrawingMutex) {
                // testing some stuffs
                if (renderPlayerEsp) {
                        for (auto& it : instance.m_Players)
                        {
                                if (const auto& player = it.playerData.validate();
                                        player.has_value()
                                        && !player.is_Disconnected()
                                        && player.is_LocalPlayer()
                                        && (!player.get_PlayerData()->fields.IsDead || State.ShowEsp_Ghosts)
                                        && it.OnScreen)
                                {
                                        if (State.ShowEsp_Box)
                                        {
                                                float width = GetScaleFromValue(35.0f);
                                                float height = GetScaleFromValue(120.0f);

                                                ImVec2 top{ it.Position.x + width, it.Position.y };
                                                ImVec2 bottom{ it.Position.x - width, it.Position.y - height };

                                                RenderBox(top, bottom, height, width, it.Color);
                                        }

                                        if (State.ShowEsp_Distance)
                                        {
                                                const ImVec2 position{ it.Position.x, it.Position.y + 15.0f * State.dpiScale };

                                                Vector2 mouse = {
                                                        ImGui::GetMousePos().x, ImGui::GetMousePos().y
                                                };

                                                std::string lel = std::to_string(ScreenToWorld(mouse).x) + ", " + std::to_string(ScreenToWorld(mouse).y);

                                                std::string lol = std::to_string(it.Position.x) + ", " + std::to_string(it.Position.y);
                                                char* player = lol.data();

                                                RenderText(player, position, it.Color);
                                        }
                                }
                        }
                }

                // Main player ESP loop
                for (auto& it : instance.m_Players)
                {
                        if (const auto& player = it.playerData.validate();
                                player.has_value()
                                && !player.is_Disconnected()
                                && !player.is_LocalPlayer()
                                && (!player.get_PlayerData()->fields.IsDead || State.ShowEsp_Ghosts)
                                && it.OnScreen)
                        {
                                /////////////////////////////////
                                //// Box ////////////////////////
                                /////////////////////////////////
                                if (State.ShowEsp_Box)
                                {
                                        float width = GetScaleFromValue(35.0f);
                                        float height = GetScaleFromValue(120.0f);

                                        ImVec2 top{ it.Position.x + width, it.Position.y };
                                        ImVec2 bottom{ it.Position.x - width, it.Position.y - height };

                                        ImVec4 boxColor = State.ShowEsp_RoleColorBox ? it.RoleColor : it.Color;
                                        RenderBox(top, bottom, height, width, boxColor);
                                }

                                /////////////////////////////////
                                //// Distance ///////////////////
                                /////////////////////////////////
                                if (State.ShowEsp_Distance)
                                {
                                        ImVec2 position = { it.Position.x, it.Position.y + 15.0f * State.dpiScale };
                                        ImVec2 position2 = { it.Position.x, it.Position.y + 30.0f * State.dpiScale };

                                        float minX = 40.0f, minY = 0.0f,
                                                maxX = DirectX::GetWindowSize().x - 46.0f,
                                                maxY = DirectX::GetWindowSize().y - 38.0f;

                                        float x = it.Position.x, y = it.Position.y;
                                        float offset = 15.0f * State.dpiScale;

                                        if (x < minX) x = minX;
                                        else if (x > maxX) x = maxX;

                                        if (y < minY) y = minY;
                                        else if (y > maxY) y = maxY;

                                        position  = { x, y + offset };
                                        position2 = { x, y + 2 * offset };

                                        char distance[32];
                                        sprintf_s(distance, "[%.0fm]", it.Distance);

                                        std::string lol = it.Name;
                                        char* playerName = lol.data();

                                        GameOptions options;
                                        if (const auto& playerVal = it.playerData.validate();
                                                State.ShowKillCD
                                                && !playerVal.get_PlayerData()->fields.IsDead
                                                && playerVal.get_PlayerData()->fields.Role
                                                && playerVal.get_PlayerData()->fields.Role->fields.CanUseKillButton)
                                        {
                                                float killTimer = playerVal.get_PlayerControl()->fields.killTimer;
                                                sprintf_s(distance, "[%.1fs]", killTimer);
                                        }

                                        RenderText(playerName, position,  it.Color);
                                        RenderText(distance,   position2, it.Color);
                                }

                                /////////////////////////////////
                                //// Tracers (HyperMenu port) ///
                                /////////////////////////////////
                                if (State.ShowEsp_Tracers)
                                {
                                        bool isDead = it.IsDead;
                                        bool isImp  = it.IsImpostor;
                                        bool shouldDraw = false;

                                        if (!isDead) {
                                                // Alive player tracers: honour crew/imp sub-toggles
                                                if (!State.ShowEsp_RoleBased) {
                                                        // No role filter active — draw for everyone
                                                        shouldDraw = true;
                                                } else {
                                                        shouldDraw = (State.ShowEsp_Tracers_Crew && !isImp)
                                                                  || (State.ShowEsp_Tracers_Imp  &&  isImp);
                                                }
                                        } else {
                                                // Ghost/dead player tracers: separate toggle (HyperMenu tracersGhosts)
                                                shouldDraw = State.ShowEsp_Tracers_Ghosts;
                                        }

                                        if (shouldDraw) {
                                                ImVec4 tracerColor = GetTracerColor(it);
                                                RenderLine(instance.LocalPosition, it.Position, tracerColor, true);
                                        }
                                }
                        }
                }
        }
}
