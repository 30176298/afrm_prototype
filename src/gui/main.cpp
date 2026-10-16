#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

#include "core/fuel.hpp"
#include "core/load.hpp"
#include "core/version.hpp"

namespace {

const ImVec4 kGreen(0.30f, 0.80f, 0.40f, 1.0f);
const ImVec4 kAmber(1.00f, 0.75f, 0.10f, 1.0f);
const ImVec4 kRed(1.00f, 0.40f, 0.40f, 1.0f);

// Everything the window shows and can edit
struct AppState {
  char flight_path[512];
  char aircraft_path[512];
  Flight flight;
  Aircraft aircraft;
  Reserves reserves;
  std::vector<float> altitude_ft;  // for the profile plot
  std::string load_error;
  bool loaded = false;
};

// Reads both files, keeps old data and shows error if present
void load_files(AppState& state) {
  std::ifstream flight_file(state.flight_path);
  std::ifstream aircraft_file(state.aircraft_path);
  if (!flight_file) {
    state.load_error = "Cannot open the flight file.";
    return;
  }
  if (!aircraft_file) {
    state.load_error = "Cannot open the aircraft file.";
    return;
  }

  Flight flight;
  Aircraft aircraft;
  std::string error = load_flight(flight_file, flight);
  if (!error.empty()) {
    state.load_error = "Flight file: " + error;
    return;
  }
  error = load_aircraft(aircraft_file, aircraft);
  if (!error.empty()) {
    state.load_error = "Aircraft file: " + error;
    return;
  }

  state.flight = flight;
  state.aircraft = aircraft;
  state.altitude_ft.clear();
  std::transform(flight.begin(), flight.end(), std::back_inserter(state.altitude_ft),
                 [](const FlightPoint& point) { return static_cast<float>(point.altitude_ft); });
  state.load_error.clear();
  state.loaded = true;
}

void draw_data(AppState& state) {
  ImGui::SeparatorText("Data files");
  ImGui::InputText("Flight profile", state.flight_path, sizeof state.flight_path);
  ImGui::InputText("Aircraft", state.aircraft_path, sizeof state.aircraft_path);
  if (ImGui::Button("Load")) load_files(state);
  if (!state.load_error.empty()) {
    ImGui::TextColored(kRed, "%s", state.load_error.c_str());
    if (state.loaded) ImGui::TextColored(kAmber, "The data below is from the previous successful load.");
  }

  if (!state.loaded) return;
  const FlightPoint& first = state.flight.front();
  const FlightPoint& last = state.flight.back();
  ImGui::Text("Flight: %d rows, %.0f NM, %.0f minutes", static_cast<int>(state.flight.size()),
              last.distance_nm - first.distance_nm, (last.time_s - first.time_s) / 60.0);
  ImGui::PlotLines("##altitude", state.altitude_ft.data(), static_cast<int>(state.altitude_ft.size()), 0,
                   "Altitude profile (0 to 45,000 ft)", 0.0f, 45000.0f, ImVec2(-1.0f, 70.0f));
}

// User settings
void draw_settings(AppState& state) {
  Aircraft& aircraft = state.aircraft;
  ImGui::SeparatorText("Aircraft");
  ImGui::Text("%s: take-off weight %.0f kg, fuel capacity %.0f kg", aircraft.model.c_str(),
              aircraft.takeoff_weight_kg, aircraft.fuel_capacity_kg);
  ImGui::InputDouble("Fuel on board (kg)", &aircraft.fuel_onboard_kg, 100.0, 500.0, "%.0f");
  ImGui::InputDouble("Stores extra burn (%)", &aircraft.stores_burn_pct, 1.0, 5.0, "%.1f");

  Reserves& reserves = state.reserves;
  ImGui::SeparatorText("Reserves");
  ImGui::InputDouble("Contingency (% of trip fuel)", &reserves.contingency_pct, 1.0, 5.0, "%.1f");
  ImGui::InputDouble("Alternate distance (NM)", &reserves.alternate_nm, 10.0, 50.0, "%.0f");
  ImGui::InputDouble("Final reserve (minutes)", &reserves.final_reserve_min, 5.0, 15.0, "%.0f");
  ImGui::InputDouble("Marginal band (% of required)", &reserves.marginal_pct, 1.0, 5.0, "%.1f");
}

void result_row(const char* name, double value, const char* unit) {
  ImGui::TableNextRow();
  ImGui::TableNextColumn();
  ImGui::TextUnformatted(name);
  ImGui::TableNextColumn();
  ImGui::Text("%.1f %s", value, unit);
}

void draw_results(const AppState& state) {
  ImGui::SeparatorText("Results");
  const Report report = assess(state.flight, state.aircraft, state.reserves);
  if (!report.error.empty()) {
    ImGui::TextColored(kRed, "Input rejected: %s", report.error.c_str());
    return;
  }

  if (ImGui::BeginTable("results", 2, ImGuiTableFlags_SizingFixedFit)) {
    result_row("Trip fuel", report.trip_kg, "kg");
    result_row("Contingency", report.contingency_kg, "kg");
    result_row("Alternate", report.alternate_kg, "kg");
    result_row("Final reserve", report.final_reserve_kg, "kg");
    result_row("Required fuel", report.required_kg, "kg");
    result_row("Fuel on board", state.aircraft.fuel_onboard_kg, "kg");
    result_row("Spare fuel (margin)", report.margin_kg, "kg");
    result_row("Maximum range", report.max_range_nm, "NM");
    ImGui::EndTable();
  }

  ImVec4 colour = kGreen;
  const char* message = "Feasible within the model.";
  if (report.status == Status::Marginal) {
    colour = kAmber;
    message = "Marginal fuel warning: the spare fuel is inside the marginal band.";
  } else if (report.status == Status::Insufficient) {
    colour = kRed;
    message = "Insufficient fuel warning: the required fuel is more than the fuel on board.";
  }
  ImGui::TextColored(colour, "Status: %s", status_text(report.status));
  ImGui::TextColored(colour, "%s", message);
}

}  // namespace

int main() {
  if (!glfwInit()) return 1;
  GLFWwindow* window = glfwCreateWindow(960, 760, "AFRM Prototype", nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // wait for display refresh

  ImGui::CreateContext();
  ImGui::GetIO().IniFilename = nullptr;
  ImGui::GetStyle().FontScaleMain = 1.2f;
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  // Start with sample data
  AppState state{};
  std::snprintf(state.flight_path, sizeof state.flight_path, "%s/flight_short.csv", AFRM_DATA_DIR);
  std::snprintf(state.aircraft_path, sizeof state.aircraft_path, "%s/aircraft_clean.csv", AFRM_DATA_DIR);
  load_files(state);

  while (!glfwWindowShouldClose(window)) {
    // Sleep until an input event (or 0.1 s)
    glfwWaitEventsTimeout(0.1);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("AFRM", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
    ImGui::Text("Aircraft Fuel and Range Management v%s (simulated data, not for operational use)",
                afrm_version());
    draw_data(state);
    if (state.loaded) {
      draw_settings(state);
      draw_results(state);
    }
    ImGui::End();

    ImGui::Render();
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
