section .data
  ; colors
ansi_colors:
  BLACK_FG          db 0x1b, "[30m", NULL_CHAR
  RED_FG            db 0x1b, "[31m", NULL_CHAR
  GREEN_FG          db 0x1b, "[32m", NULL_CHAR
  YELLOW_FG         db 0x1b, "[33m", NULL_CHAR
  BLUE_FG           db 0x1b, "[34m", NULL_CHAR
  CYAN_FG           db 0x1b, "[36m", NULL_CHAR
  WHITE_FG          db 0x1b, "[37m", NULL_CHAR
  GRAY_FG           db 0x1b, "[90m", NULL_CHAR
  BRIGHT_RED_FG     db 0x1b, "[91m", NULL_CHAR
  BRIGHT_GREEN_FG   db 0x1b, "[92m", NULL_CHAR
  BRIGHT_YELLOW_FG  db 0x1b, "[93m", NULL_CHAR
  BRIGHT_BLUE_FG    db 0x1b, "[94m", NULL_CHAR
  BRIGHT_CYAN_FG    db 0x1b, "[96m", NULL_CHAR
  BRIGHT_WHITE_FG   db 0x1b, "[97m", NULL_CHAR
  DEFAULT_FG        db 0x1b, "[39m", NULL_CHAR
ansi_colors_end:

  CHANGE_COLOR_LEN equ DEFAULT_FG - BRIGHT_WHITE_FG
