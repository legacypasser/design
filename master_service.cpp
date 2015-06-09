#include "stdafx.h"
#include "master_service.h"

////////////////////////////////////////////////////////////////////////////////
// ÅäÖÃÄÚÈÝÏî

char *var_cfg_str;
acl::master_str_tbl var_conf_str_tab[] = {
	{ "str", "test_msg", &var_cfg_str },

	{ 0, 0, 0 }
};

int  var_cfg_bool;
acl::master_bool_tbl var_conf_bool_tab[] = {
	{ "bool", 1, &var_cfg_bool },

	{ 0, 0, 0 }
};

int  var_cfg_int;
acl::master_int_tbl var_conf_int_tab[] = {
	{ "int", 120, &var_cfg_int, 0, 0 },

	{ 0, 0 , 0 , 0, 0 }
};

long long int  var_cfg_int64;
acl::master_int64_tbl var_conf_int64_tab[] = {
	{ "int64", 120, &var_cfg_int64, 0, 0 },

	{ 0, 0 , 0 , 0, 0 }
};



////////////////////////////////////////////////////////////////////////////////

master_service::master_service()
{
    htable = acl_htable_create(1, 0);
}

master_service::~master_service()
{

}

ACL_HTABLE* master_service::htable = acl_htable_create(1024, 0);
bool master_service::add(struct Onliner* one)
{
    int result = acl_htable_enter_r(htable, one->id, one, NULL, NULL);
    if(result == 0)
        return true;
    else
        return false;

}

struct Onliner* master_service::get(const char* id)
{
    return (struct Onliner*)acl_htable_find(htable, id);
}

bool master_service::remove(const char* id)
{
    int result = acl_htable_delete(htable, id, NULL);
    if(result == 0)
        return true;
    else
        return false;
}


void master_service::on_read(acl::socket_stream* stream)
{
	int   n;
	char  buf[1024];
    char id[8];
    char target[8];
    char timeChar[16];
    int i;
    int pos;
    time_t currentTime;
    struct Onliner* client;
	if ((n = stream->read(buf, sizeof(buf), false)) == -1)
		return;
    for(i = 6;i < n&& buf[i] != ';'; i++)
        {
            id[i - 6] = buf[i];
        }
    id[i - 6] = 0;
    switch(buf[4]){
    case '1':
        client = (struct Onliner*)get(id);
        if(client == NULL)
            {
                client = new struct Onliner;
                strcpy(client->id, id);
                strcpy(client->addr, stream->get_peer(true));
                time(&(client->last));
                add(client);
            }
        else{
                strcpy(client->addr, stream->get_peer(true));	
                time(&(client->last));
	   }
	return;
    case '2':
        i++;
        for(pos = 0; buf[i] != ';'; i++, pos++)
            target[pos] = buf[i];
        target[pos] = 0;
        time(&currentTime);
        client = (struct Onliner*)get(target);
        if(client != NULL && currentTime - client->last < 120){
            stream->set_peer(client->addr);
		logger("should direct %s", buf);
        }else{
		logger("should http %s", buf);
            i++;
            for(pos = 0; buf[i] != ';'; i++, pos++)
                timeChar[pos] = buf[i];
            timeChar[pos] = 0;
            n = snprintf(buf, sizeof(buf), "00004;%s", timeChar);
        }
        break;
    case '3':
        client = (struct Onliner*)get(id);
        stream->set_peer(client->addr);
        break;
	case '4':
	client = (struct Onliner*)get(id);
	remove(id);
	break;	

    }
	stream->write(buf, n);
}

void master_service::proc_on_init()
{
}

void master_service::proc_on_exit()
{
}
