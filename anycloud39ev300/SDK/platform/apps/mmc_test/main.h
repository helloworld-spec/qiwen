int main(int argc, char** argv);
int help( );
int open_disk(char *dev);
void release_disk(int fd);
off64_t get_disk_size(int fd);
void prog_exit( int i_sig );
int umount_disk( );
int remount_disk( );
//int is_dev_file(const char *file_path);
int print_key_value_result( );
int init_res( );
int get_shell_res( char *pc_cmd , char *pc_result , int i_len_res );
int if_mount_disk( );