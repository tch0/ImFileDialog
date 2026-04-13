#pragma once

#include <memory>
#include <ctime>
#include <stack>
#include <string>
#include <thread>
#include <vector>
#include <functional>
#include <filesystem>
#include <unordered_map>
#include <algorithm> // std::min, std::max

namespace ifd {
	// Convert char8_t* to const char* for ImGui/OS APIs
	inline const char* u8_as_char(const char8_t* s) {
		return reinterpret_cast<const char*>(s);
	}
	// Convert std::string (UTF-8 encoding) to std::u8string
	inline std::u8string to_u8string(const std::string& s) {
		return std::u8string(s.begin(), s.end());
	}
	// Convert const char* (UTF-8 encoding) to std::u8string
	inline std::u8string to_u8string(const char* s) {
		return std::u8string(reinterpret_cast<const char8_t*>(s));
	}
	// Convert std::u8string to std::string (UTF-8 bytes as char)
	inline std::string u8_to_string(const std::u8string& s) {
		return std::string(s.begin(), s.end());
	}
	
	enum class Format: char{
		BGRA,
		RGBA,
		RGB
	};

	class FileDialog {
	public:
		static inline FileDialog& getInstance()
		{
			static FileDialog ret;
			return ret;
		}

		~FileDialog();
		
		// std::u8sting version: all u8strings are UTF-8 encoding
		// std::string version: all strings are assumed encoded with UTF-8
		bool save(const std::u8string& key, const std::u8string& title, const std::u8string& filter, const std::u8string& startingDir = u8"");
		bool save(const std::string& key, const std::string& title, const std::string& filter, const std::string& startingDir = "");
		
		bool open(const std::u8string& key, const std::u8string& title, const std::u8string& filter, bool isMultiselect = false, const std::u8string& startingDir = u8"");
		bool open(const std::string& key, const std::string& title, const std::string& filter, bool isMultiselect = false, const std::string& startingDir = "");

		bool isDone(const std::u8string& key);
		bool isDone(const std::string& key);

		inline bool hasResult() { return m_result.size(); }
		inline const std::filesystem::path& getResult() { return m_result[0]; }
		inline const std::vector<std::filesystem::path>& getResults() { return m_result; }

		void close();

		void removeFavorite(const std::u8string& path);
		void removeFavorite(const std::string& path);
		
		void addFavorite(const std::u8string& path);
		void addFavorite(const std::string& path);
		
		inline const std::vector<std::u8string>& getFavorites() { return m_favorites; }

		inline void setZoom(float z) { 
			m_zoom = std::min<float>(MAX_ZOOM_LEVEL, std::max<float>(MIN_ZOOM_LEVEL, z)); 
			m_refreshIconPreview();
		}
		inline float getZoom() { return m_zoom; }

		std::function<void*(const uint8_t*, int, int, Format)> createTexture;
		std::function<void(void*)> deleteTexture;

	private:
		static constexpr auto MAX_ZOOM_LEVEL = 25.0f;
		static constexpr auto MIN_ZOOM_LEVEL = 1.0f;

		enum class DialogType {
			openFile,
			openDirectory,
			saveFile
		};

		struct FileTreeNode {
#ifdef _WIN32
			FileTreeNode(const std::wstring& path) {
				this->path = std::filesystem::path(path);
				read = false;
			}
#endif
			FileTreeNode(const std::u8string& path) {
				this->path = std::filesystem::path(path);
				read = false;
			}

			std::filesystem::path path;
			bool read;
			std::vector<std::unique_ptr<FileTreeNode>> children;
		};

		struct SmartSize {
			SmartSize() = default;
			SmartSize(size_t s);

			auto operator<=>(const SmartSize& others) const noexcept
			{
				return sizeInByte <=> others.sizeInByte;
			}

			size_t sizeInByte;
			float size;
			std::u8string unit;
		};

		struct FileData {
			FileData(const std::filesystem::path& path);

			std::filesystem::path path;
			bool isDirectory;
			SmartSize size;
			time_t dateModified;

			bool hasIconPreview;
			void* iconPreview;
			uint8_t* iconPreviewData;
			int iconPreviewWidth, iconPreviewHeight;
		};

		std::u8string m_currentKey;
		std::u8string m_currentTitle;
		std::filesystem::path m_currentDirectory;
		bool m_isMultiselect;
		bool m_isOpen;
		DialogType m_type;
		std::u8string m_inputTextbox;
		std::u8string m_pathBuffer;
		std::u8string m_newEntryBuffer;
		std::u8string m_searchBuffer;
		std::vector<std::u8string> m_favorites;
		bool m_calledOpenPopup;
		std::stack<std::filesystem::path> m_backHistory;
		std::stack<std::filesystem::path> m_forwardHistory;
		float m_zoom;
		std::vector<std::filesystem::path> m_selections;
		int m_selectedFileItem;
		std::vector<std::filesystem::path> m_result;
		std::u8string m_filter;
		std::vector<std::vector<std::u8string>> m_filterExtensions;
		size_t m_filterSelection;
		std::unordered_map<std::u8string, void*> m_icons;
		std::thread m_previewLoader;
		bool m_previewLoaderRunning;
		std::vector<std::unique_ptr<FileTreeNode>> m_treeCache;
		unsigned int m_sortColumn;
		unsigned int m_sortDirection;
		std::vector<FileData> m_content;
		bool confirmationPopup = false;
		std::unordered_map<std::u8string, std::unordered_map<std::u8string, std::filesystem::path>> m_iconPathCache;
		
		FileDialog();
		void m_select(const std::filesystem::path& path, bool isCtrlDown = false);
		bool m_finalize(const std::u8string& filename = u8"");
		void m_parseFilter(const std::u8string& filter);
		void* m_getIcon(const std::filesystem::path& path);
		void m_loadDefaultIcon(const std::filesystem::path& path, const std::u8string& pathU8);
		void m_clearIcons();
		void m_refreshIconPreview();
		void m_clearIconPreview();
		void m_stopPreviewLoader();
		void m_loadPreview();
		void m_renderTree(FileTreeNode& node);
		void m_setDirectory(const std::filesystem::path& p, bool addHistory = true);
		void m_sortContent(unsigned int column, unsigned int sortDirection);
		void m_renderContent();
		void m_renderPopups();
		void m_renderFileDialog();

#ifdef __linux__
		std::filesystem::path m_locateIcon(const std::u8string& iconName, int size);
#endif
	};
}
