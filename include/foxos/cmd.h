#pragma once

void __libterm_send_command_ps2(char* command, int size);
void __libterm_send_command(char* command, int size);
void __libterm_shutdown_hook();