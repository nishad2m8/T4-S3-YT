SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    ui_theme_manager.c
    ui_themes.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_img_icon_tv_png.c
    images/ui_img_img_icon_tv_off_png.c
    images/ui_img_img_icon_ac_png.c
    images/ui_img_img_icon_ac_on_png.c
    images/ui_img_img_icon_lamp1_png.c
    images/ui_img_img_icon_lamp_on_png.c
    images/ui_img_636281941.c
    images/ui_img_img_icon_washer_png.c
    images/ui_img_img_icon_washer_off_png.c
    images/ui_img_img_icon_cleaner_png.c
    images/ui_img_img_icon_cleaner_off_png.c
    images/ui_img_img_icon_plug_png.c
    images/ui_img_img_icon_plug_on_png.c
    images/ui_img_img_icon_car_png.c
    images/ui_img_img_icon_car_on_png.c
    images/ui_img_img_icon_car_2_png.c
    fonts/ui_font_Kanit25.c
    fonts/ui_font_Kanit35.c
    fonts/ui_font_Kanit50.c
    fonts/ui_font_Kanit60.c)

add_library(ui ${SOURCES})
