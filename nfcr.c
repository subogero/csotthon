#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int c2hex(unsigned char c, unsigned char *buffer)
{
	char lsn = c & 0x0F;
	char msn = c >> 4;
	buffer[0] = msn < 10 ? msn + 48 : msn + 65;
	buffer[1] = lsn < 10 ? lsn + 48 : lsn + 65;
	buffer[2] = 0;
}

int read_nfc(char *dev, int tags)
{
	int fd = open(dev, O_RDONLY);
	if (fd < 0) {
		static const char *err = "Unable to open ";
		write(2, err, strlen(err));
		write(2, dev, strlen(dev));
		write(2, "\n", 1);
		return fd;
	}
	unsigned char nfc[16];
	unsigned char hex[4];
	int tag_on = 0;
	int n_tags = 0;
	int n_nulls = 0;
	while (1) {
		int bytes = read(fd, nfc, 16);
		if (bytes == 0) {
			static const char *err = "Unable to read ";
			write(2, err, strlen(err));
			write(2, dev, strlen(dev));
			write(2, "\n", 1);
			return 1;
		}

		if (bytes == 2 && nfc[0] == 0 && nfc[1] == 0) {
			if (tag_on && n_nulls++ >= 3) {
				write(1, "\n", 1);
				tag_on = 0;
				if (tags && ++n_tags >= tags)
					return 0;
			}
			continue;
		}
		n_nulls = 0;
		tag_on = 1;

		int i;
		for (i = 0; i < bytes; ++i) {
			c2hex(nfc[i], hex);
			write(1, hex, strlen(hex));
		}
	}
	return 0;
}

void help(void)
{
	static const char *helptext =
"Usage:\n"
"\tnfcr [options]\n"
"\n"
"Description: read NFC tags and print them to STDOUT in HEX format\n"
"\n"
"Options:\n"
"\t-h\tPrint this help text and exit\n"
"\t-d <n>\tNumber of /dev/hidraw<n> device to use, default 2\n"
"\t-n <n>\tNumber of tags to read, default go on forever\n"
	;
	write(1, helptext, strlen(helptext));
	_exit(0);
}

int main(int argc, char *argv[])
{
	int tags = 0;
	char ndev[1024] = "2";
	int i = 1;
	while (i < argc) {
		if (strcmp(argv[i], "-h") == 0)
			help();
		else if (strcmp(argv[i], "-d") == 0)
			strcpy(ndev, argv[++i]);
		else if (strcmp(argv[i], "-n") == 0)
			tags = atoi(argv[++i]);
		i++;
	}
	char dev[1024] = "/dev/hidraw";
	strcat(dev, ndev);
	return read_nfc(dev, tags);
}
