#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char *path)
{
  static char buf[DIRSIZ + 1]; // 声明一个静态字符数组，长度为DIRSIZ+1，用于存储处理后的文件名。
  char *p;                     // 声明一个字符指针，用于遍历字符串。

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--) // 从字符串的末尾向前寻找第一个斜杠（'/'）。指针先指向path，然后移动strlen(path)个位置，也就是指向path的末尾
    ;
  p++; // 移动指针到斜杠后的第一个字符，即文件名的开始。

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ) // 如果文件名的长度大于或等于DIRSIZ，则直接返回p指向的字符串。
    return p;
  memmove(buf, p, strlen(p));                       // 将文件名复制到buf数组中。
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p)); // 在文件名后填充空格，直到达到DIRSIZ的长度。
  return buf;                                       // 返回处理后的文件名字符串。
}

void ls(char *path)
{
  char buf[512], *p; // 声明一个足够大的字符数组用于存储路径，和一个字符指针。
  int fd;            // 文件描述符。
  struct dirent de;  // 目录条目结构体。
  struct stat st;    // 文件状态结构体。

  if ((fd = open(path, 0)) < 0)
  {                                           // 尝试打开指定的路径，获取文件描述符。
    fprintf(2, "ls: cannot open %s\n", path); // 如果打开失败，打印错误信息到标准错误。
    return;                                   // 返回。
  }

  if (fstat(fd, &st) < 0)
  {                                           // 获取文件描述符指向的文件的状态。
    fprintf(2, "ls: cannot stat %s\n", path); // 如果获取失败，打印错误信息。
    close(fd);                                // 关闭文件描述符。
    return;                                   // 返回。
  }

  switch (st.type)
  {                                                                   // 根据文件类型进行不同的处理。
  case T_FILE:                                                        // 如果是文件。
    printf("%s %d %d %l\n", fmtname(path), st.type, st.ino, st.size); // 打印文件名，类型，inode号和大小。
    break;

  case T_DIR: // 如果是目录。
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
    {                                // 检查路径长度是否超出buf大小。
      printf("ls: path too long\n"); // 如果超出，打印错误信息。
      break;
    }
    strcpy(buf, path);     // 将路径复制到buf。
    p = buf + strlen(buf); // 设置指针p到buf的末尾。
    *p++ = '/';            // 在末尾添加斜杠。
    while (read(fd, &de, sizeof(de)) == sizeof(de))
    {                              // 读取目录项。确保只有在每次成功读取一个完整的目录项时，循环才会继续
      if (de.inum == 0)            // 如果目录项无效（inode号为0）。
        continue;                  // 忽略。
      memmove(p, de.name, DIRSIZ); // 将目录项的名字复制到p指向的位置。
      p[DIRSIZ] = 0;               // 设置字符串结束符。
      if (stat(buf, &st) < 0)
      {                                      // 获取完整路径的文件状态。
        printf("ls: cannot stat %s\n", buf); // 如果获取失败，打印错误信息。
        continue;                            // 继续下一项。
      }
      printf("%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size); // 打印文件名，类型，inode号和大小。
    }
    break;
  }
  close(fd); // 关闭文件描述符。
}
int main(int argc, char *argv[])
{
  int i; // 用于循环的变量。

  if (argc < 2)
  {          // 如果命令行参数少于2个（即没有提供路径）。
    ls("."); // 调用ls函数，列出当前目录的内容。
    exit(0); // 退出程序。
  }
  for (i = 1; i < argc; i++) // 遍历所有提供的路径。
    ls(argv[i]);             // 调用ls函数，列出指定路径的内容。
  exit(0);                   // 退出程序。
}
