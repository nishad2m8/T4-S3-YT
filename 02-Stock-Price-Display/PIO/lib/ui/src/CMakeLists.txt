SET(SOURCES screens/ui_Screen.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_images_mask_png.c
    images/ui_img_images_open_png.c
    images/ui_img_images_high_png.c
    images/ui_img_images_low_png.c
    images/ui_img_images_volume_png.c
    images/ui_img_images_timer_png.c
    fonts/ui_font_ka25.c
    fonts/ui_font_ka30.c
    fonts/ui_font_ka55.c
    fonts/ui_font_ka90.c)

add_library(ui ${SOURCES})
