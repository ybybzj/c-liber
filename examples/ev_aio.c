#include <common/dbg.h>
#include <system/nx.h>
// #include <system/nx/io/io.h>
#include <fevent_loop/event_loop.h>

static void open_cb(int, int, ev_monitor*);
static ssize_t read_cb(int,int,void*,size_t,void*,ev_monitor*);
int main(int argc, char *argv[])
{
	validate_cls_args(argc, argv, 2, "file-name");
	ev_monitor *monitor = ev_monitor_create();
	check(monitor != NULL, goto onerr);
	int i = 1;
	for(;i < argc; i++)
		ev_fs_open(monitor, argv[i], O_RDWR|O_EXCL, FILE_MODE, open_cb);

	check(ev_loop_run(monitor ,0) != -1, goto onerr);
	// check(ev_loop_run(monitor ,EV_EMPTY_NOEXIT) != -1, goto onerr);
	ev_monitor_free(monitor);
	onerr:
	exit(EXIT_FAILURE);

}



static void open_cb(int err, int fd, ev_monitor *monitor)
{
	if(err)
		println ("Error opening file: %s",strerror(err));
	else
	{
		struct stat fs;
		check(fstat(fd, &fs) != -1, return);
		println("File[%d] opened, size : %ld", fd, (long)fs.st_size);
		int *count = MALLOC(1,int);
		*count = 0;
		ev_fs_read(monitor, fd, read_cb, (void*)count);
	}
}

static ssize_t read_cb(int err, int fd, void *buf, size_t buf_size,void *arg UNUSED, ev_monitor *monitor UNUSED)
{
	int *count = (int*)arg;
	if(err)
	{
		println ("Error reading file: %s",strerror(err));
		close(fd);
		return -1;
	}
	else{
		if(buf != NULL)
		{
			char *str = buf;
			int size = buf_size > 50 ? 50 : (int)buf_size;
			*count += size;
			// sleep(1);
			println("[read_cb %d %d] : %.*s",fd, *count, size, str);
			return (ssize_t)size;
		}else
		{
			debug_B("- EOF - [size of %d]: %d", fd, *count);
			close(fd);
			free(count);
			return -1;
		}
	}
}
