#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void msg(char *line)
{
	int len = strlen(line);
	write(2, line, len);
	if (line[len - 1] != '\n')
		write(2, "\n", 1);
}

void debug(char *line)
{
	static on = 0;
	if (strncmp(line, "DEBUG ON!", 10) == 0)
		on = 1;
	if (!on)
		return;
	msg(line);
}

int chexcat(unsigned char c, unsigned char *str)
{
	char lsn = c & 0x0F;
	char msn = c >> 4;
	char buffer[3];
	buffer[0] = msn < 10 ? msn + 48 : msn + 65;
	buffer[1] = lsn < 10 ? lsn + 48 : lsn + 65;
	buffer[2] = 0;
	strcat(str, buffer);
}

int read_nfc(char *dev, int tags)
{
	int fd = open(dev, O_RDONLY);
	if (fd < 0) {
		char err[128] = "Unable to open ";
		strcat(err, dev);
		msg(err);
		return fd;
	}
	unsigned char nfc[16];
	unsigned char hex[128] = "";
	int tag_on = 0;
	int n_tags = 0;
	int n_nulls = 0;
	while (1) {
		int bytes = read(fd, nfc, 16);
		int i;
		char hexinput[16] = "";
		for (i = 0; i < bytes; ++i) {
			chexcat(nfc[i], hexinput);
		}
		debug(hexinput);
		if (bytes == 0) {
			char err[128] = "Unable to read ";
			strcat(err, dev);
			msg(err);
			return 1;
		}

		if (bytes == 2 && nfc[0] == 0 && nfc[1] == 0) {
			if (tag_on && n_nulls++ >= 5) {
				if (strlen(hex) > 10) {
					strcat(hex, "\n");
					write(1, hex, strlen(hex));
					hex[0] = 0;
				}
				tag_on = 0;
				if (tags && ++n_tags >= tags)
					return 0;
			}
			continue;
		}
		n_nulls = 0;
		tag_on = 1;

		strcat(hex, hexinput);
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
"\t-v\tVerbose debug messages on STDERR\n"
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
		else if (strcmp(argv[i], "-v") == 0)
			debug("DEBUG ON!");
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
