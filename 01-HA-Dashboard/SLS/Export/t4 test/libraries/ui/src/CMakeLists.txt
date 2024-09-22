SET(SOURCES components/ui_comp_widget.c
    components/ui_comp.c
    screens/ui_Screen1.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_1100325711.c)

add_library(ui ${SOURCES})
