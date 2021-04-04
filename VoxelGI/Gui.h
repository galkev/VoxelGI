#pragma once

#include <string>
#include <sstream>
#include <functional>
#include <iostream>
#include <string>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <algorithm>
#include "imgui\imgui.h"

#include "ticpp/ticpp.h"

namespace VoxelGI
{
	class Gui
	{
	private:
		static std::string filename;
		static bool onLoad;

		static bool onSave;
		static bool saveRequested;

		static ticpp::Document doc;
		static ticpp::Element* curElement;

		template <typename T>
		static bool loadValue(const std::string& name, T& val)
		{
			return true;
		}

		template <typename T>
		static bool storeValue(const std::string& name, T& val)
		{
			curElement->SetAttribute(name, val);
			curElement = curElement->NextSiblingElement();
			return true;
		}

		template <typename T>
		static void calcMinMaxAvg(const std::vector<T>& data, T& minVal, T& maxVal, T& avgVal)
		{
			minVal = std::numeric_limits<T>::infinity();
			maxVal = -std::numeric_limits<T>::infinity();
			avgVal = (T)0.0;

			for (int i = 0; i < data.size(); i++)
			{
				minVal = std::min(data[i], minVal);
				maxVal = std::max(data[i], maxVal);
				avgVal += data[i];
			}

			avgVal /= data.size();
		}

		template <typename T>
		static void printMinMaxAvg(const std::vector<T>& data)
		{
			T minVal, maxVal, avgVal;

			calcMinMaxAvg(data, minVal, maxVal, avgVal);

			std::stringstream text;

			text.precision(20);

			text << "min: " << minVal << "; max: " << maxVal << "; avg: " << avgVal;

			ImGui::Text(text.str().c_str());
		}

		template <typename T, bool (*func) (const char*, T* val)>
		static bool display(const std::string& name, T& val)
		{
			if (onSave)
				storeValue(name, val);

			return func(name.c_str(), &val);
		}

		static void BeginSave()
		{
			if (saveRequested)
			{
				try
				{
					//doc.LoadFile();
					doc.Clear();

					onSave = true;
				}
				catch (ticpp::Exception& ex)
				{
					std::cout << "Exception opening settings file for save" << std::endl;
				}

				saveRequested = false;
			}
		}

		static void EndSave()
		{
			if (onSave)
			{
				doc.SaveFile(filename);
			}

			onSave = false;
		}

		static void BeginLoad()
		{
			onLoad = true;
		}

		static void EndLoad()
		{
			onLoad = false;
		}

	public:
		static void SaveNextFrame()
		{
			saveRequested = true;
		}

		static void Begin(const std::string& name)
		{
			ImGui::Begin(name.c_str());
			BeginSave();
		}

		static void End()
		{
			EndSave();

			ImGui::End();
		}

		static void SameLine()
		{
			ImGui::SameLine();
		}

		static bool Button(const std::string& name)
		{
			return ImGui::Button(name.c_str());
		}

		static void Text(const std::string& name)
		{
			ImGui::Text(name.c_str());
		}

		static void PlotLines(const std::string& name, const std::vector<float>& data, bool minMaxAvg = false)
		{
			ImGui::PlotLines(name.c_str(), data.data(), data.size());
			
			if (minMaxAvg)
				printMinMaxAvg(data);
		}

		static void PlotLines(const std::string& name, const std::vector<double>& data, bool minMaxAvg = false)
		{
			ImGui::PlotLines(name.c_str(), std::vector<float>(data.begin(), data.end()).data(), data.size());

			if (minMaxAvg)
				printMinMaxAvg(data);
		}

		static bool Property(const std::string& name, bool& val)
		{
			//return display<bool, ImGui::Checkbox>(name, val);
			return ImGui::Checkbox(name.c_str(), &val);
		}

		static bool Property(const std::string& name, int& val)
		{
			return ImGui::InputInt(name.c_str(), &val);
		}

		static bool Property(const std::string& name, float& val)
		{
			return ImGui::InputFloat(name.c_str(), &val);
		}

		static bool Property(const std::string& name, double& val)
		{
			float val2 = val;
			bool changed = Property(name, val2);
			val = val2;
			return changed;
		}

		static bool Property(const std::string& name, glm::ivec2& val)
		{
			return ImGui::InputInt2(name.c_str(), glm::value_ptr(val));
		}

		static bool Property(const std::string& name, glm::vec2& val)
		{
			return ImGui::InputFloat2(name.c_str(), glm::value_ptr(val));
		}

		static bool Property(const std::string& name, glm::vec3& val)
		{
			return ImGui::InputFloat3(name.c_str(), glm::value_ptr(val));
		}

		static bool Property(const std::string& name, glm::dvec3& val)
		{
			glm::vec3 val2 = val;
			bool changed = Property(name, val2);
			val = val2;
			return changed;
		}

		static bool Property(const std::string& name, glm::ivec3& val)
		{
			return ImGui::InputInt3(name.c_str(), glm::value_ptr(val));
		}

		template <typename E>
		static bool Property(const std::string& name, E& val, std::vector<const char*> values)
		{
			return ImGui::Combo(name.c_str(), (int*)&val, values.data(), values.size());
		}

		template <size_t valueCount, typename E>
		static bool Property(const std::string& name, E& val, const char* values[valueCount])
		{
			return ImGui::Combo(name.c_str(), (int*)&val, values, valueCount);
		}

		template <typename T, typename V>
		static bool Property(const std::string& name, T* obj, const V val, void (T::*setter)(V val))
		{
			V valNew = val;
			if (Property(name, valNew))
			{
				(obj->*setter)(valNew);
				return true;
			}
			else
				return false;
		}

		template <typename T, typename G, typename S>
		static bool Property(const std::string& name, T* obj, G getter, void (T::*setter)(S))
		{
			auto val = (obj->*getter)();
			if (Property(name, val))
			{
				(obj->*setter)(val);
				return true;
			}
			else
				return false;
		}

		/*template <typename V>
		static bool Property(const std::string& name, V(*getter)(), void(*setter)(V val))
		{
			V val = getter();
			if (Property(name, val))
			{
				setter(val);
				return true;
			}
			else
				return false;
		}*/
	};
}
