#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *
fmtname(char *path)
{
  static char buf[DIRSIZ + 1]; // 创建一个静态字符数组，长度为DIRSIZ+1（通常是文件名最大长度+1）。
  char *p;

  // Find first character after last slash.
  for (p = path + strlen(path); p >= path && *p != '/'; p--) // 从路径末尾向前搜索，直到找到第一个斜杠（'/'）。
    ;
  p++; // 移动到斜杠后的第一个字符，通常是文件名的开始。

  // Return blank-padded name.
  if (strlen(p) >= DIRSIZ) // 如果文件名长度大于或等于DIRSIZ，直接返回p。
    return p;
  memmove(buf, p, strlen(p));                     // 将文件名复制到buf中。
  memset(buf + strlen(p), 0, DIRSIZ - strlen(p)); // 用0填充剩余部分，意味着在字符串末尾添加终止符。
  return buf;                                     // 返回处理后的文件名。
}

int norecurse(char *path)
{
  char *buf = fmtname(path);        // 获取路径的文件名。
  if (buf[0] == '.' && buf[1] == 0) // 如果是当前目录("."), 则不需要递归。
    return 1;
  if (buf[0] == '.' && buf[1] == '.' && buf[2] == 0) // 如果是上级目录(".."), 也不需要递归。
    return 1;
  return 0; // 其他情况需要递归。
}

void find(char *path, char *target)
{
  char buf[512], *p; // 创建缓冲区和字符指针。
  int fd;            // 文件描述符。
  struct dirent de;  // 目录项结构。
  struct stat st;    // 文件状态结构。

  if ((fd = open(path, 0)) < 0) // 尝试打开指定路径。
  {
    fprintf(2, "find: cannot open %s\n", path); // 打开失败，输出错误信息。
    return;
  }

  if (fstat(fd, &st) < 0) // 获取文件状态。
  {
    fprintf(2, "find: cannot stat %s\n", path); // 获取失败，输出错误信息并关闭文件描述符。
    close(fd);
    return;
  }

  if (strcmp(fmtname(path), target) == 0) // 检查文件名是否为目标文件名。
  {
    printf("%s\n", path); // 如果是，打印路径。
  }

  switch (st.type) // 根据文件类型处理。
  {
  case T_FILE: // 如果是文件类型，不做特殊处理。
    break;

  case T_DIR:                                       // 如果是目录类型。
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) // 检查路径长度是否超出缓冲区。
    {
      printf("find: path too long\n"); // 超出长度，输出错误信息。
      break;
    }
    strcpy(buf, path);                              // 复制路径到缓冲区。
    p = buf + strlen(buf);                          // 移动指针到缓冲区末尾。
    *p++ = '/';                                     // 添加斜杠。
    while (read(fd, &de, sizeof(de)) == sizeof(de)) // 读取目录项。这个while循环的意义就在于，对当前目录下的每一项都进行读取。
    {
      if (de.inum == 0) // 如果目录项无效，继续下一个。
        continue;
      memmove(p, de.name, DIRSIZ); // 复制目录项名称到路径末尾。
      p[DIRSIZ] = 0;               // 确保路径字符串正确终止。
      if (stat(buf, &st) < 0)      // 获取新路径的文件状态。
      {
        printf("find: cannot stat %s\n", buf); // 获取失败，输出错误信息。
        continue;
      }
      if (norecurse(buf) == 0) // 检查是否需要递归。
      {
        find(buf, target); // 递归查找。
      }
    }
    break;
  }
  close(fd); // 关闭文件描述符。
}
int main(int argc, char *argv[])
{
  if (argc == 1) // 如果没有提供任何参数。
  {
    printf("usage:find [path] [target] \n"); // 打印用法。
    exit(0);                                 // 退出程序。
  }
  if (argc == 2) // 如果只提供了一个参数。
  {
    find(".", argv[1]); // 在当前目录下查找。
    exit(0);
  }
  if (argc == 3) // 如果提供了两个参数。
  {
    find(argv[1], argv[2]); // 在指定目录下查找。
    exit(0);
  }
  exit(0); // 保证程序能够退出。
}
