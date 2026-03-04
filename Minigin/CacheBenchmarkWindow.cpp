#include "CacheBenchmarkWindow.h"
#include <imgui.h>
#include <chrono>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <cstdio>

namespace
{
    // ── Struct definitions ──────────────────────────────────────────────
    struct Transform
    {
        float matrix[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
    };
    // each object is 68 bytes (> 64-byte cache line).
    struct GameObject3D
    {
        Transform transform;
        int ID = 0;
    };
    // Iterating only over IDs is now identical to iterating over plain ints,
    struct GameObject3DAlt
    {
        std::vector<int> IDs;
        std::vector<Transform> transforms;

        void resize(size_t n)
        {
            IDs.assign(n, 1);
            transforms.resize(n);
        }
        size_t size() const { return IDs.size(); }
    };

    // ── Generic helpers ─────────────────────────────────────────────────
    template <typename T, typename Accessor>
    void RunStrideKernel(T *data, size_t count, int step, Accessor &&accessor)
    {
        for (size_t i = 0; i < count; i += static_cast<size_t>(step))
            accessor(data[i]);
    }
    constexpr size_t kTargetBytes = static_cast<size_t>(1) << 28; // 256 MB
    constexpr size_t kIntCount = kTargetBytes / sizeof(int);

    template <typename RunFunc>
    void MeasureStrideTimings(RunFunc &&runFunc, int numSamples,
                              std::vector<int> *stepsOut, std::vector<float> &timingsOut)
    {
        if (stepsOut)
            stepsOut->clear();
        timingsOut.clear();
        for (int step = 1; step <= 1024; step *= 2)
        {
            std::vector<long long> samples;
            samples.reserve(static_cast<size_t>(numSamples));

            for (int s = 0; s < numSamples; ++s)
            {
                const auto start = std::chrono::high_resolution_clock::now();
                runFunc(step);
                const auto end = std::chrono::high_resolution_clock::now();
                samples.push_back(
                    std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
            }
            std::sort(samples.begin(), samples.end());
            // Trim top & bottom outlier
            long long sum = std::accumulate(samples.begin() + 1, samples.end() - 1, 0LL);
            float avgUs = static_cast<float>(sum) / static_cast<float>(numSamples - 2);

            if (stepsOut)
                stepsOut->push_back(step);
            timingsOut.push_back(avgUs / 1000.f); // µs → ms
        }
    }

    // ── Exercise 1: plain int buffer ────────────────────────────────────
    void RenderExercise1Window()
    {
        static int numSamples = 10;
        static bool done = false;
        static std::vector<int> steps;
        static std::vector<float> timings;

        ImGui::Begin("Exercise 1");

        ImGui::InputInt("# samples", &numSamples);
        if (numSamples < 3)
            numSamples = 3;

        if (ImGui::Button("Thrash the cache"))
        {
            std::vector<int> buf(kIntCount, 1);
            auto run = [&buf](int step)
            {
                RunStrideKernel(buf.data(), buf.size(), step,
                                [](int &v)
                                { v *= 2; });
            };
            MeasureStrideTimings(run, numSamples, &steps, timings);
            done = true;
        }

        if (done && !timings.empty())
        {
            ImGui::Separator();

            float maxVal = *std::max_element(timings.begin(), timings.end());
            char overlay[64];
            std::snprintf(overlay, sizeof(overlay), "max: %.2f ms", maxVal);
            ImGui::PlotHistogram("##cacheInts", timings.data(),
                                 static_cast<int>(timings.size()), 0, overlay,
                                 0.f, maxVal * 1.1f,
                                 ImVec2(ImGui::GetContentRegionAvail().x, 80));

            ImGui::Spacing();
            if (ImGui::BeginTable("int_results", 2,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                      ImGuiTableFlags_SizingStretchProp))
            {
                ImGui::TableSetupColumn("Step size", ImGuiTableColumnFlags_WidthFixed, 80.f);
                ImGui::TableSetupColumn("Time (ms)");
                ImGui::TableHeadersRow();
                for (size_t i = 0; i < timings.size(); ++i)
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", steps[i]);
                    ImGui::TableSetColumnIndex(1);
                    float ratio = timings[i] / maxVal;
                    ImVec4 color = ratio < 0.33f   ? ImVec4(0.2f, 1.f, 0.2f, 1.f)
                                   : ratio < 0.66f ? ImVec4(1.f, 1.f, 0.2f, 1.f)
                                                   : ImVec4(1.f, 0.3f, 0.3f, 1.f);
                    ImGui::TextColored(color, "%.2f", timings[i]);
                }
                ImGui::EndTable();
            }
        }
        ImGui::End();
    }

    // ── Exercise 2: GameObject3D vs GameObject3DAlt ────────
    void RenderExercise2Window()
    {
        static int numSamples = 10;
        static std::vector<int> steps;
        static std::vector<float> timingsObj;
        static std::vector<float> timingsAlt;
        static bool doneObj = false;
        static bool doneAlt = false;

        ImGui::Begin("Exercise 2");

        ImGui::InputInt("# samples##ex2", &numSamples);
        if (numSamples < 3)
            numSamples = 3;

        // ── Button 1: GameObject3D ──
        if (ImGui::Button("Thrash the cache with GameObject3D"))
        {
            const size_t count = std::max<size_t>(1, kTargetBytes / sizeof(GameObject3D));
            std::vector<GameObject3D> buf(count);
            for (size_t i = 0; i < count; ++i)
                buf[i].ID = static_cast<int>(i);

            auto run = [&buf](int step)
            {
                RunStrideKernel(buf.data(), buf.size(), step,
                                [](GameObject3D &o)
                                { o.ID *= 2; });
            };
            MeasureStrideTimings(run, numSamples, &steps, timingsObj);
            doneObj = true;
        }

        // ── Button 2: GameObject3DAlt ──
        if (ImGui::Button("Thrash the cache with GameObject3DAlt"))
        {
            const size_t count = std::max<size_t>(1, kTargetBytes / sizeof(GameObject3D));
            GameObject3DAlt soa;
            soa.resize(count);

            // Iterate only over the contiguous ID array – same as iterating ints
            auto run = [&soa](int step)
            {
                RunStrideKernel(soa.IDs.data(), soa.IDs.size(), step,
                                [](int &id)
                                { id *= 2; });
            };
            MeasureStrideTimings(run, numSamples, nullptr, timingsAlt);
            doneAlt = true;
        }

        // ── Visualisation ──────────────────────────────────────────────
        if ((doneObj || doneAlt) && (!timingsObj.empty() || !timingsAlt.empty()))
        {
            ImGui::Separator();
            float maxObj = timingsObj.empty() ? 0.f : *std::max_element(timingsObj.begin(), timingsObj.end());
            float maxAlt = timingsAlt.empty() ? 0.f : *std::max_element(timingsAlt.begin(), timingsAlt.end());
            float combinedMax = std::max(maxObj, maxAlt) * 1.1f;
            if (doneObj && !timingsObj.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 0.4f, 0.3f, 1.0f));
                char ov[64];
                std::snprintf(ov, sizeof(ov), "GameObject3D  (max %.2f ms)", maxObj);
                ImGui::PlotHistogram("##go3d", timingsObj.data(),
                                     static_cast<int>(timingsObj.size()), 0, ov,
                                     0.f, combinedMax,
                                     ImVec2(ImGui::GetContentRegionAvail().x, 60));
                ImGui::PopStyleColor();
            }
            if (doneAlt && !timingsAlt.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.3f, 0.9f, 0.4f, 1.0f));
                char ov[64];
                std::snprintf(ov, sizeof(ov), "GameObject3DAlt  (max %.2f ms)", maxAlt);
                ImGui::PlotHistogram("##go3dalt", timingsAlt.data(),
                                     static_cast<int>(timingsAlt.size()), 0, ov,
                                     0.f, combinedMax,
                                     ImVec2(ImGui::GetContentRegionAvail().x, 60));
                ImGui::PopStyleColor();
            }
            ImGui::Spacing();
            if (doneObj && doneAlt && timingsObj.size() == timingsAlt.size())
            {
                if (ImGui::BeginTable("go_results", 4,
                                      ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                          ImGuiTableFlags_SizingStretchProp))
                {
                    ImGui::TableSetupColumn("Step", ImGuiTableColumnFlags_WidthFixed, 60.f);
                    ImGui::TableSetupColumn("AoS (ms)");
                    ImGui::TableSetupColumn("SoA (ms)");
                    ImGui::TableSetupColumn("Speedup");
                    ImGui::TableHeadersRow();

                    for (size_t i = 0; i < timingsObj.size(); ++i)
                    {
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        ImGui::Text("%d", steps[i]);
                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%.2f", timingsObj[i]);
                        ImGui::TableSetColumnIndex(2);
                        ImGui::Text("%.2f", timingsAlt[i]);
                        ImGui::TableSetColumnIndex(3);
                        float speedup = timingsObj[i] / std::max(timingsAlt[i], 0.0001f);
                        ImVec4 c = speedup > 1.1f
                                       ? ImVec4(0.3f, 0.9f, 0.4f, 1.0f)
                                       : ImVec4(0.9f, 0.9f, 0.9f, 1.0f);
                        ImGui::TextColored(c, "%.2fx", speedup);
                    }
                    ImGui::EndTable();
                }
            }
        }
        ImGui::End();
    }
}

void dae::CacheBenchmarkWindow::RenderImGui()
{
    RenderExercise1Window();
    RenderExercise2Window();
}
