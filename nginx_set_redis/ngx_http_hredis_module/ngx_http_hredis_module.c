/*
 * Copyright (C) Eric Zhang
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <hiredis/hiredis.h>

/* Module config */
typedef struct {
    ngx_int_t  ed;
} ngx_http_echo_loc_conf_t;

static ngx_str_t  ngx_http_redis_key = ngx_string("redis_key");
static void saveIpToRedis();
static char *ngx_http_echo(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_echo_create_loc_conf(ngx_conf_t *cf);
static ngx_int_t ngx_http_log_init(ngx_conf_t *cf);
static ngx_int_t ngx_http_echo_handler(ngx_http_request_t *r);

/* Directives */
static ngx_command_t  ngx_http_echo_commands[] = {
        { ngx_string("echo"),
          NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
          ngx_http_echo,
          NGX_HTTP_LOC_CONF_OFFSET,
          offsetof(ngx_http_echo_loc_conf_t, ed),
          NULL },

        ngx_null_command
};

/* Http context of the module */
static ngx_http_module_t  ngx_http_echo_module_ctx = {
        NULL,                                  /* preconfiguration */
        ngx_http_log_init,                     /* postconfiguration */

        NULL,                                  /* create main configuration */
        NULL,                                  /* init main configuration */

        NULL,                                  /* create server configuration */
        NULL,                                  /* merge server configuration */

        ngx_http_echo_create_loc_conf,         /* create location configration */
        NULL,                                  /* merge location configration */
};

/* Module */
ngx_module_t  ngx_http_echo_module = {
        NGX_MODULE_V1,
        &ngx_http_echo_module_ctx,             /* module context */
        ngx_http_echo_commands,                /* module directives */
        NGX_HTTP_MODULE,                       /* module type */
        NULL,                                  /* init master */
        NULL,                                  /* init module */
        NULL,                                  /* init process */
        NULL,                                  /* init thread */
        NULL,                                  /* exit thread */
        NULL,                                  /* exit process */
        NULL,                                  /* exit master */
        NGX_MODULE_V1_PADDING
};

static ngx_int_t
ngx_http_log_init(ngx_conf_t *cf)
{
    ngx_http_core_main_conf_t  *cmcf;
    ngx_http_handler_pt        *h;
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);

    h = ngx_array_push(&cmcf->phases[NGX_HTTP_LOG_PHASE].handlers);
    if (h == NULL) {
        return NGX_ERROR;
    }
    *h = ngx_http_echo_handler;
    return NGX_OK;
}

/* Handler function */
static ngx_int_t
ngx_http_echo_handler(ngx_http_request_t *r)
{
    ngx_http_echo_loc_conf_t *elcf;
    elcf = ngx_http_get_module_loc_conf(r, ngx_http_echo_module);
    ngx_http_variable_value_t* key=ngx_http_get_indexed_variable(r, elcf->ed);
    ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "just the test %s",key->data);
    saveIpToRedis();
    return NGX_OK;
}

static void saveIpToRedis(){
    redisContext *conn  = redisConnect("127.0.0.1",6379);
    if(conn != NULL && conn->err) {
    }else {
        redisReply *reply = (redisReply *) redisCommand(conn, "set foo 1234");
        freeReplyObject(reply);
        redisFree(conn);
    }
}

static char *
ngx_http_echo(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_echo_loc_conf_t  *echoClcf=conf;
    echoClcf->ed= ngx_http_get_variable_index(cf, &ngx_http_redis_key);
    return NGX_CONF_OK;
}

static void *
ngx_http_echo_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_echo_loc_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_echo_loc_conf_t));
    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    conf->ed=0;

    return conf;
}


