
set(app_sources
"main.c"
"./eve/EVE_commands.c"
"./eve/EVE_target.c"
"tft.c"
"tft_data.c"
"ads1232.c"
"scspiffs.c"
"mfrc522.c"
"wifi_station.c"
"sntp.c"
"http.c"
"hangle.c"
"serialLED.c"
"./color/color.c"
"./lib8tion/lib8tion.c"
"intro.c"
"w5500.c"
"ui_wifi_scan.c"
"ui_fail.c"
"ui_cali.c"
"file_server.c"
)
set(EXTRA_COMPONENT_DIRS ./esp-idf-lib/components)
idf_component_register(SRCS ${app_sources}
EMBED_TXTFILES howsmyssl_com_root_cert.pem
                    postman_root_cert.pem
EMBED_FILES "favicon.ico" "upload_script.html")




