#include "console_helper.hpp"
#include "error_defs.hpp"
#include "gui.hpp"

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) ((a > b) ? a : b)
#define CLAMP(n, l, h) (MIN(MAX(n, l), h))

void ModGui::scrollSelection(int delta) {
    if (delta == 0) {
        return;
    }

    size_t new_selection = CLAMP((ssize_t) (selected_row + delta), 0, mod_list.size() - 1);
    if (new_selection == selected_row) {
        return;
    }

    size_t new_scroll;
    if (delta < 0) {
        new_scroll = MIN(scroll, new_selection);
    } else {
        new_scroll = MAX(scroll, new_selection - MIN(new_selection, display_rows - 1));
    }

    if (new_scroll == scroll && false) {
        redrawRow(listToGuiSpace(selected_row));
        redrawRow(listToGuiSpace(new_selection));
        
        selected_row = new_selection;
    } else {
        selected_row = new_selection;
        scroll = new_scroll;
        redraw();
    }
}

void ModGui::redraw(void) {
    for (size_t y = 0; y < MIN(display_rows, mod_list.size()); y++) {
        redrawRow(y);
    }
}

void ModGui::redrawRow(size_t gui_y) {
    if (gui_y >= display_rows) {
        PANIC();
    }

    size_t screen_y = guiToScreenSpace(gui_y);
    size_t list_index = guiToListSpace(gui_y);

    if (list_index < 0 || list_index >= mod_list.size()) {
        PANIC();
    }

    std::shared_ptr<SkyrimMod> cur_mod = mod_list.at(list_index);

    bool highlighted = selected_row == list_index;

    CONSOLE_SET_POS(0, 0);
    for (size_t i = 0; i < screen_y; i++) {
        CONSOLE_MOVE_DOWN(1);
    }
    CONSOLE_CLEAR_LINE();

    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
    printf("[");

    ModStatus mod_status = cur_mod->getStatus();
    switch (mod_status) {
        case ModStatus::ENABLED:
            CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_GREEN);
            printf("*");
            break;
        case ModStatus::PARTIAL:
            CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_YELLOW);
            printf("*");
            break;
        case ModStatus::DISABLED:
            printf(" ");
            break;
        default:
            PANIC();
    }

    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
    printf("] ");

    if (highlighted) {
        CONSOLE_SET_ATTRS(CONSOLE_ATTR_NONE);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_BLACK);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_BG_WHITE);
    } else {
        CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
        CONSOLE_SET_COLOR(CONSOLE_COLOR_BG_BLACK);
    }

    printf(cur_mod->base_name.c_str());
    printf("\n");

    CONSOLE_SET_ATTRS(CONSOLE_ATTR_BOLD);
    CONSOLE_SET_COLOR(CONSOLE_COLOR_BG_BLACK);
    CONSOLE_SET_COLOR(CONSOLE_COLOR_FG_WHITE);
}
