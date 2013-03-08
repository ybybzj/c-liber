#define _GNU_SOURCE
#include <common/dbg.h>
#include <system/nx.h>
#include <system/nx/net/inet_utils.h>
#include <system/nx/io/io.h>
#include <system/nx/io/ring_buffer.h>
#include <system/nx/sigutils.h>
#include <event_loop/event_loop.h>

#define SERV_PORT "5001"

#define MAX_BUF 1024

// static volatile sig_atomic_t gotSigint = 0;

static ev_monitor *monitor = NULL;

static void handler(int sig UNUSED);

static ev_set on_listen(event, ev_monitor *);
static ev_set on_read(event, ev_monitor *);
static ev_set on_write(event, ev_monitor *);


int main()
{
	ERR_CLEAN_INIT();
	int listenfd;
	
	check(SIG_HDL_REG(emptysigset(), 0, handler, SIGINT, SIGTERM, SIGQUIT) == 0, goto onerr);

	monitor = ev_monitor_create();
	check(monitor != NULL, goto onerr);
	ERR_CLEAN_PUSH(0, monitor, monitor);
	check((listenfd = inet_tcp_listen(NULL, SERV_PORT, NULL)) != -1, goto onerr);
	ERR_CLEAN_PUSH(1,&listenfd,listenfd);

	event listen_ev;
	memset(&listen_ev,0,sizeof(event));
	listen_ev.fd = listenfd;
	listen_ev.events = EV_READ;
	check(ev_monitor_ctl(monitor, EV_CTL_ADD, listen_ev, EV_CB(EV_READ,on_listen)) != -1, goto onerr);
	println(" - listening -");

	check(ev_loop_run(monitor ,EV_EMPTY_NOEXIT) != -1, goto onerr);

	ev_monitor_free(monitor);
	exit_s(true);

onerr:
	ERR_CLEAN_BEGIN
	case 0:
	{
		
		ev_monitor_free((ev_monitor *)cleanup_resource);
		break;
	}
	case 1:
	{
		int *fdp = (int*)cleanup_resource;
		check(close(*fdp) != -1, terminate(true));
		break;
	}
	ERR_CLEAN_END
	ev_monitor_free(monitor);
	terminate(true);

}


static void handler(int sig)
{
	int  savedErrno;

	savedErrno = errno;
	if(sig == SIGINT || sig == SIGTERM || sig == SIGQUIT)
	{
		ev_loop_stop(monitor);
	}

	errno = savedErrno;
	return;
}

static void message_free(void *buf)
{
	(void)ring_buffer_free((ring_buffer_t*)buf);
}

static ev_set on_listen(event ev, ev_monitor *monitor)
{
	
	int sockfd;
	errno = 0;
	ev_set events = ev.events;
	if((sockfd = accept(ev.fd, NULL, NULL)) < 0)
	{
		if(errno == EINTR || errno == ECONNABORTED || errno == EWOULDBLOCK || errno == EPROTO)
		{
			if(errno == EINTR)
				println("[accept] signal caught!!!");
			return events;
		}
		else
		{
			println("stopping event loop...");
			ev_loop_stop(monitor);
			return EV_FINISHED;
		}
	}
	

	char cl_addr_str[INET_ADDR_STR_LEN];
	println("client %s is connected...",peer_atos(sockfd, cl_addr_str, INET_ADDR_STR_LEN));

	ring_buffer_t *message_buffer = ring_buffer_create(8*(1<<10));
	check(message_buffer != NULL, return 0);
	ev.fd = sockfd;
	ev.events = EV_READ;
	ev.priority = 1;
	ev.data.ptr = message_buffer;
	ev.ev_data_free = message_free;

	check(ev_monitor_ctl(monitor, EV_CTL_ADD, ev, EV_CB(EV_READ, on_read), EV_CB(EV_WRITE, on_write)) != -1, return events);

	return events;
}


static int get_message(char *buf, size_t len)
{
	char *p = buf;
	while(p != (buf+len - 1))
	{
		if(*p == '\r' && *(p+1) == '\n')
			break;
		p++;
	}
	return p != (buf+len - 1);
}

static ev_set on_read(event ev, ev_monitor *monitor)
{

	println("[fd %d]: read happened!",ev.fd);
	ssize_t n;
    ring_buffer_t *message_buffer = (ring_buffer_t*)ev.data.ptr;
    ev_set events = ev.events;
    errno = 0;
  	
  	if(ring_buffer_free_space(message_buffer) == 0)
  		return EV_FINISHED;
  	char *wp = (char*)ring_buffer_write_pos(message_buffer);
    if((n = read(ev.fd, ring_buffer_write_pos(message_buffer), ring_buffer_free_space(message_buffer))) > 0)
    {
    	ring_buffer_commit_write(message_buffer, n);
       	if(ring_buffer_free_space(message_buffer) == 0 || get_message(wp,n))
       	{
    		println("{get message}");
       		
   	    	events |= EV_WRITE;
       	}
    	return ring_buffer_free_space(message_buffer) ? events : (events & ~EV_READ);
    }
    else
    {
    	if (errno == EINTR)
   		{
			println("[read] signal caught!!!");
			return events;
		}
    	else
    	{
    		if(n == 0)
		    	println("peer socket is closed!");
		    else
				print_err("read from socket failed!");

			if(ring_buffer_data_count(message_buffer) == 0)
			{
				check(ev_monitor_ctl(monitor, EV_CTL_DEL, ev) != -1, return EV_FINISHED);
			}
			check_err(close(ev.fd) != -1, return EV_FINISHED, "sockfd close failed!");
			println("socket %d is closing...",ev.fd);


			return EV_FINISHED;
    	}

    }
}

static ssize_t make_output(void *in, size_t in_len, void *out, size_t out_len)
{
	char *p = (char *)in;
	int i = 0;
	while(p != (in + in_len-1))
	{	
		if(*p == '\r' && *(p+1) == '\n')
			break;
		i++;
		p++;
	}
	if(p != (in + in_len-1))
		snprintf((char*)out,out_len,"{message %d} %.*s\n", i, i, (char *)in);
	else
		snprintf((char*)out,out_len,"{message incomplete}");
	return p == (in + in_len-1) ? 0 : i + 2;
}

static ev_set on_write(event ev, ev_monitor *monitor)
{
	println("[fd %d]: write happened!",ev.fd);

	ssize_t n;
	ring_buffer_t *message_buffer = (ring_buffer_t*)ev.data.ptr;
	char out[1024] = {'\0'};
	ev_set events = ev.events;

    errno = 0;

    if(ring_buffer_data_count(message_buffer) == 0)
    {
    	
    	return EV_FINISHED;
    }

    ssize_t readn = 0;
    if((readn = make_output(ring_buffer_read_pos(message_buffer), ring_buffer_data_count(message_buffer), out, 1024)) < 0)
    	snprintf(out,1024,"make response failed!");
    else
    {
    	ring_buffer_commit_read(message_buffer, readn);
    }

    if((n = write(ev.fd, out, 1024)) > 0)
    {
    	
    	if(ring_buffer_free_space(message_buffer) > 0)
	  	{
			events |= EV_READ;
	  	}
	  	
	  	return ring_buffer_data_count(message_buffer) && readn >= 0 ? events : (events&~EV_WRITE);
    }else{
    	if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
		{
			return events;
		}else{
			print_err("write to socket failed!");
			check(ev_monitor_ctl(monitor, EV_CTL_DEL, ev) != -1, return EV_FINISHED);
			check_err(close(ev.fd) != -1, return EV_FINISHED, "sockfd close failed!");
			return EV_FINISHED;

		}

    }

}