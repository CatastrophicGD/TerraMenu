#include "pch-il2cpp.h"
#include "esp_tab.h"
#include "game.h"
#include "state.hpp"
#include "utility.h"
#include "gui-helpers.hpp"

namespace EspTab {

        void Render() {
                bool changed = false;
                ImGui::SameLine(100 * State.dpiScale);
                ImGui::BeginChild("###ESP", ImVec2(500 * State.dpiScale, 0), true, ImGuiWindowFlags_NoBackground);

                /////////////////////////////
                // General
                /////////////////////////////
                changed |= ToggleButton("Enable",                 &State.ShowEsp);
                changed |= ToggleButton("Show Ghosts",            &State.ShowEsp_Ghosts);
                changed |= ToggleButton("Hide During Meetings",   &State.HideEsp_During_Meetings);
                changed |= ToggleButton("Show Boxes",             &State.ShowEsp_Box);
                changed |= ToggleButton("Show Distances",         &State.ShowEsp_Distance);

                changed |= ToggleButton("Role-based",             &State.ShowEsp_RoleBased);
                if (State.ShowEsp_RoleBased) {
                        ImGui::SameLine();
                        changed |= ToggleButton("Crewmates",  &State.ShowEsp_Crew);
                        ImGui::SameLine();
                        changed |= ToggleButton("Impostors",  &State.ShowEsp_Imp);
                }

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                /////////////////////////////
                // Tracers  (HyperMenu port)
                /////////////////////////////
                BoldText("Tracers", State.MenuThemeColor);
                changed |= ToggleButton("Show Tracers",           &State.ShowEsp_Tracers);

                if (State.ShowEsp_Tracers) {
                        ImGui::Spacing();

                        // Per-role tracer toggles (mirrors HyperMenu tracersCrew / tracersImps / tracersGhosts)
                        changed |= ToggleButton("Crewmate Tracers",   &State.ShowEsp_Tracers_Crew);
                        ImGui::SameLine();
                        changed |= ToggleButton("Impostor Tracers",   &State.ShowEsp_Tracers_Imp);
                        ImGui::SameLine();
                        changed |= ToggleButton("Ghost Tracers",      &State.ShowEsp_Tracers_Ghosts);

                        ImGui::Spacing();

                        // Tracer color mode (mirrors HyperMenu colorBasedTracers / distanceBasedTracers)
                        if (ToggleButton("Color-based Tracers",       &State.ShowEsp_ColorBasedTracers)) {
                                if (State.ShowEsp_ColorBasedTracers) {
                                        State.ShowEsp_DistanceBasedTracers = false;
                                        State.ShowEsp_RoleColorTracers = false;
                                }
                                changed = true;
                        }
                        ImGui::SameLine();
                        if (ToggleButton("Distance-based Tracers",    &State.ShowEsp_DistanceBasedTracers)) {
                                if (State.ShowEsp_DistanceBasedTracers) {
                                        State.ShowEsp_ColorBasedTracers = false;
                                        State.ShowEsp_RoleColorTracers = false;
                                }
                                changed = true;
                        }
                        ImGui::SameLine();
                        if (ToggleButton("Role Color Tracers",        &State.ShowEsp_RoleColorTracers)) {
                                if (State.ShowEsp_RoleColorTracers) {
                                        State.ShowEsp_ColorBasedTracers = false;
                                        State.ShowEsp_DistanceBasedTracers = false;
                                }
                                changed = true;
                        }
                        if (State.ShowEsp_DistanceBasedTracers) {
                                ImGui::TextDisabled("  Red = close  |  Yellow = medium  |  Green = far");
                        }
                }

                ImGui::Spacing();

                /////////////////////////////
                // Box color
                /////////////////////////////
                BoldText("Box", State.MenuThemeColor);
                changed |= ToggleButton("Role Color Box",             &State.ShowEsp_RoleColorBox);
                ImGui::SameLine();
                ImGui::TextDisabled("Colors boxes by player role");

                ImGui::EndChild();
                if (changed) {
                        State.Save();
                }
        }
}
