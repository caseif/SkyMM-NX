#pragma once

#include "mod.hpp"

#include <memory>
#include <string>
#include <vector>

class ModGui {
    private:
        std::vector<std::shared_ptr<SkyrimMod>> &mod_list;
        size_t screen_off_y;
        size_t display_rows;
        size_t selected_row;
        size_t scroll;

        inline size_t listToGuiSpace(size_t list_index) {
            return list_index - scroll;
        }

        inline size_t listToScreenSpace(size_t list_index) {
            return guiToScreenSpace(listToGuiSpace(list_index));
        }

        inline size_t guiToListSpace(size_t gui_y) {
            return gui_y + scroll;
        }

        inline size_t guiToScreenSpace(size_t gui_y) {
            return gui_y + screen_off_y;
        }

        inline size_t screenToListSpace(size_t screen_y) {
            return guiToListSpace(screenToGuiSpace(screen_y));
        }

        inline size_t screenToGuiSpace(size_t screen_y) {
            return screen_y - screen_off_y;
        }

    public:
        ModGui(std::vector<std::shared_ptr<SkyrimMod>> &mod_list, size_t screen_off_y, size_t display_rows):
                mod_list(mod_list),
                screen_off_y(screen_off_y),
                display_rows(display_rows),
                selected_row(0),
                scroll(0) {
        }

        void scrollSelection(int delta);

        void redraw(void);

        void redrawRow(size_t row);
};
