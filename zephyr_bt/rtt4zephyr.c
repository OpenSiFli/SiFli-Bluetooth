#include "rtt4zephyr.h"
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/services/ias.h>
#include "zephyr/settings/settings.h"
#include <../host/conn_internal.h>
#include <../host/l2cap_internal.h>
#include <zephyr/sys_clock.h>
#include <zephyr/sys/iterable_sections.h>
#include <zephyr/console/console.h>

#ifdef BT_MESH
    #include <zephyr/bluetooth/mesh.h>
    #include <mesh/subnet.h>
    #include <mesh/mesh.h>
#endif

#ifdef SETTINGS
    #include <zephyr/settings/settings.h>
#endif


#include "bf0_sibles.h"
#include <stdio.h>
#include <rtdef.h>
#include <stdarg.h>

k_timepoint_t sys_timepoint_calc(k_timeout_t timeout)
{
    k_timepoint_t r;

    r.tick = rt_tick_get() + timeout.ticks;
    return r;
}

k_timeout_t sys_timepoint_timeout(k_timepoint_t timepoint)
{
    k_timeout_t r;

    r.ticks = timepoint.tick - rt_tick_get();
    return r;
}

bool device_is_ready(const struct device *dev)
{
    return 1;
}


/**********************************************************************************/

#if 0
static RNG_HandleTypeDef rng_hdl;
int bt_rand(void *buf, size_t len)
{
    uint8_t *buf2 = (uint8_t *)buf;

    rng_hdl.Instance = hwp_trng;
    HAL_RNG_Init(&rng_hdl);
    while (len)
    {
        uint32_t val;
        HAL_RNG_Generate(&rng_hdl, &val, 0);
        if (len > sizeof(uint32_t))
        {
            memcpy(buf2, &val, sizeof(uint32_t));
            buf2 += sizeof(uint32_t);
            len -= sizeof(uint32_t);
        }
        else
        {
            memcpy(buf2, &val, len);
            break;
        }
    }
    return 0;
}

int bt_encrypt_le(const uint8_t key[16], const uint8_t plaintext[16],
                  uint8_t enc_data[16])
{
    HAL_AES_init((uint32_t *)key, 16, NULL, AES_MODE_ECB);
    HAL_AES_run(1, (uint8_t *)plaintext, (uint8_t *)enc_data, 16);
    return 0;
}
#endif

rt_tick_t k_work_delayable_remaining_get(const struct rt_delayed_work *dwork)
{
    rt_tick_t t = 0;

    if (dwork->timer.timeout_tick > rt_tick_get())
        t = dwork->timer.timeout_tick - rt_tick_get();

    return t;
}

int k_work_reschedule2(struct rt_delayed_work *dwork, rt_tick_t delay)
{
    k_timeout_t t;
    int r;

    if (dwork->work.list.next == NULL && dwork->work.list.prev == NULL)
        rt_work_init(&(dwork->work), dwork->work.work_func, dwork->work.work_data);

    t.ticks = delay;
    if (dwork->workqueue)
        r = rt_workqueue_cancel_work(dwork->workqueue, &(dwork->work));
    r = k_work_schedule(dwork, t);
    return r;
}

int k_mutex_init(struct rt_mutex *mutex)
{
    static int idx = 0;
    int r = RT_EOK;
    char name[8];

    if (mutex->parent.parent.type == 0)
    {
        rt_sprintf(name, "k_mux%d", idx++);
        r = rt_mutex_init(mutex, name, RT_IPC_FLAG_FIFO);
    }
    else
        mutex->value = 1;
    return r;
}


int k_mutex_lock(struct k_mutex *mutex, k_timeout_t timeout)
{
    int r = RT_EOK;

    if (mutex->parent.parent.type == 0)
    {
        k_mutex_init(mutex);
    }
    r = rt_mutex_take(mutex, timeout.ticks);
    return r;
}


int k_sem_init(struct k_sem *sem, unsigned int initial_count, unsigned int limit)
{
    static int idx = 0;
    char name[8];

    rt_sprintf(name, "k_sem%d", idx++);
    rt_sem_init(sem, name, initial_count, RT_IPC_FLAG_FIFO);
    return 0;
}

int k_sem_take(struct k_sem *sem, k_timeout_t timeout)
{
    int r = RT_EOK;

    if (sem->parent.parent.type == 0)
        k_sem_init(sem, sem->value, 0);
    r = rt_sem_take(sem, timeout.ticks);
    if (r == -RT_ETIMEOUT)
        r = -EAGAIN;
    return r;
}

void k_sem_give(struct k_sem *sem)
{
    if (sem->parent.parent.type == 0)
        k_sem_init(sem, sem->value, 0);
    rt_sem_release(sem);
}


/*********************work*********************************************/

/*Fix me: implement need review.*/
bool k_work_flush(struct k_work *work,
                  struct k_work_sync *sync)
{
    bool need_flush = (work->flags & RT_WORK_STATE_PENDING);

    /* If necessary wait until the flusher item completes */
    while (need_flush)
    {
        rt_thread_delay(20);
        need_flush = (work->flags & RT_WORK_STATE_PENDING);
    }
    return need_flush;
}

/***********************slab memory*********************************/

static void slab_init(struct rt_mempool *slab)
{
    char *name = (char *)slab->start_address;
    uint32_t size = (slab->block_size + sizeof(rt_uint8_t *)) * slab->block_total_count;
    uint8_t *m = rt_malloc(size);
    rt_mp_init(slab, name, m, size, slab->block_size);
}

int k_mem_slab_alloc(struct rt_mempool *slab, void **mem,
                     k_timeout_t timeout)
{

    if (slab->size == 0)
    {
        slab_init(slab);
    }
    *mem = rt_mp_alloc(slab, timeout.ticks);
    if (*mem == NULL)
        return -ENOMEM;
    else
        return RT_EOK;
}

void k_mem_slab_free(struct rt_mempool *slab, void *mem)
{
    rt_mp_free(mem);
}


uint32_t k_mem_slab_num_free_get(struct k_mem_slab *slab)
{
    if (slab->size == 0)
    {
        slab_init(slab);
    }
    return slab->block_free_count;
}

#ifdef __ARMCC_VERSION
    __USED struct net_buf_pool _net_buf_pool_list_start SPACE1("._net_buf_pool.static.00_start");
    __USED struct net_buf_pool _net_buf_pool_list_end SPACE1("._net_buf_pool.static.zz_end");

    #ifdef BT_CONN
        __USED const struct bt_conn_cb _bt_conn_cb_list_start SPACE1("._bt_conn_cb.static.00_start");
        __USED const struct bt_conn_cb _bt_conn_cb_list_end SPACE1("._bt_conn_cb.static.zz_end");
        __USED const struct bt_l2cap_fixed_chan _bt_l2cap_fixed_chan_list_start SPACE1("._bt_l2cap_fixed_chan.static.00_start");
        __USED const struct bt_l2cap_fixed_chan _bt_l2cap_fixed_chan_list_end  SPACE1("._bt_l2cap_fixed_chan.static.zz_end");
        __USED const struct bt_gatt_service_static _bt_gatt_service_static_list_start  SPACE1("._bt_gatt_service_static.static.00_start");
        __USED const struct bt_gatt_service_static _bt_gatt_service_static_list_end  SPACE1("._bt_gatt_service_static.static.zz_end");
    #endif

    #ifdef BT_IAS
        __USED const struct bt_ias_cb _bt_ias_cb_list_start  SPACE1("._bt_ias_cb.static.00_start");
        __USED const struct bt_ias_cb _bt_ias_cb_list_end  SPACE1("._bt_ias_cb.static.zz_end");
    #endif

    #ifdef SETTINGS
        __USED const struct settings_handler_static _settings_handler_static_list_start  SPACE1("._settings_handler_static.static.00_start");
        __USED const struct settings_handler_static _settings_handler_static_list_end  SPACE1("._settings_handler_static.static.zz_end");
    #endif


    #ifdef BT_MESH
        __USED const struct bt_mesh_subnet_cb _bt_mesh_subnet_cb_list_start SPACE1("._bt_conn_cb.static.00_start");
        __USED const struct bt_mesh_subnet_cb  _bt_mesh_subnet_cb_list_end SPACE1("._bt_conn_cb.static.zz_end");
        __USED const struct bt_mesh_app_key_cb  _bt_mesh_app_key_cb_list_start SPACE1("._bt_mesh_app_key_cb.static.00_start");
        __USED const struct bt_mesh_app_key_cb  _bt_mesh_app_key_cb_list_end SPACE1("._bt_mesh_app_key_cb.static.zz_end");
        __USED const struct bt_mesh_hb_cb    _bt_mesh_hb_cb_list_start SPACE1("._bt_mesh_hb_cb.static.00_start");
        __USED const struct bt_mesh_hb_cb    _bt_mesh_hb_cb_list_end SPACE1("._bt_mesh_hb_cb.static.zz_end");
        __USED const struct bt_mesh_friend_cb    _bt_mesh_friend_cb_list_start SPACE1("._bt_mesh_friend_cb.static.00_start");
        __USED const struct bt_mesh_friend_cb    _bt_mesh_friend_cb_list_end SPACE1("._bt_mesh_friend_cb.static.zz_end");
        __USED const struct bt_mesh_proxy_cb    _bt_mesh_proxy_cb_list_start SPACE1("._bt_mesh_proxy_cb.static.00_start");
        __USED const struct bt_mesh_proxy_cb    _bt_mesh_proxy_cb_list_end SPACE1("._bt_mesh_proxy_cb.static.zz_end");
    #endif
#endif

/********************FIFO ***********************************************/
void k_fifo_put(struct k_fifo *fifo, void *data)
{
    struct rt_slist_node *node = (struct rt_slist_node *)data;

    rt_enter_critical();
    if (node && fifo)
    {
        node->next = NULL;
        if (fifo->tail == NULL)
            fifo->tail = node;
        else
        {
            fifo->tail->next = node;
            fifo->tail = node;
        }
        if (fifo->head == NULL)
            fifo->head = node;
    }
    rt_exit_critical();
}

void *k_fifo_get(struct k_fifo *fifo, k_timeout_t timeout)
{
    struct rt_slist_node *r = NULL;

    rt_enter_critical();
    if (fifo && fifo->head)
    {
        r = fifo->head;
        fifo->head = r->next;
        if (r->next == NULL && fifo->tail)
            fifo->tail = NULL;
    }
    rt_exit_critical();
    return (void *)r;
}


/********************LIFO ***********************************************/
void k_lifo_put(struct k_lifo *lifo, void *data)
{
    struct rt_slist_node *node = (struct rt_slist_node *)data;

    if (node && lifo)
    {
        if (lifo->head == NULL)
        {
            node->next = NULL;
            lifo->head = node;
        }
        else
        {
            node->next = lifo->head;
            lifo->head = node;
        }
    }
}


void *k_lifo_get(struct k_lifo *lifo, k_timeout_t timeout)
{
    struct rt_slist_node *r = NULL;

    if (lifo && lifo->head)
    {
        r = lifo->head;
        lifo->head = r->next;
    }
    return (void *)r;
}

int k_work_submit(struct k_work *work)
{
    if (work->list.next == NULL && work->list.prev == NULL)
        rt_work_init(work, work->work_func, work->work_data);
    return rt_work_submit(work, 0);
}

int k_work_schedule(struct k_work_delayable *dwork,
                    k_timeout_t delay)
{
    if (dwork->work.list.next == NULL && dwork->work.list.prev == NULL)
        rt_delayed_work_init(dwork, dwork->work.work_func, dwork->work.work_data);
    return rt_work_submit((struct rt_work *)dwork, delay.ticks);
}

void k_work_queue_start(struct k_work_q *queue,
                        void *stack,
                        rt_size_t stack_size,
                        int prio,
                        const struct k_work_queue_config *cfg)
{
    if (cfg)
        rt_workqueue_start(queue, (cfg)->name, stack, stack_size, prio);
    else
        rt_workqueue_start(queue, "unknown", stack, stack_size, prio);
}

k_tid_t k_thread_create(struct k_thread *new_thread,
                        uint32_t *stack,
                        rt_size_t stack_size,
                        k_thread_entry_t entry,
                        void *p1, void *p2, void *p3,
                        int prio, uint32_t options, k_timeout_t delay)

{
    k_tid_t r = NULL;
    if (RT_EOK == rt_thread_init(new_thread, "thread", (void(*)(void *))entry, p1, stack, stack_size, prio, 20))
        r = new_thread;
    return r;
}

int k_work_submit_to_queue(struct k_work_q *queue, struct k_work *work)
{
    if (work->list.next == NULL && work->list.prev == NULL)
        rt_work_init(work, work->work_func, work->work_data);
    return rt_workqueue_submit_work(queue, work, 0);
}


int k_poll_signal_raise(struct k_poll_signal *sig, int result)
{
    if (sig->waiting_list.prev == NULL && sig->waiting_list.next == NULL)
        rt_wqueue_init(sig);
    rt_wqueue_wakeup(sig, &result);
    return 0;
}


__syscall int k_poll(struct k_poll_event *events, int num_events,
                     k_timeout_t timeout)
{
    int r = 0;

    if (events->type == K_POLL_TYPE_SIGNAL)
    {
        struct k_poll_signal *sig = events->signal;
        if (sig->waiting_list.prev == NULL && sig->waiting_list.next == NULL)
            rt_wqueue_init(sig);
        rt_wqueue_wait(events->signal, 0, timeout.ticks);
        if (rt_get_errno() == -RT_ETIMEOUT)
        {
            r = -EAGAIN;
            rt_set_errno(RT_EOK);
        }
        else
        {
            events->state = _POLL_STATE_SIGNALED;
        }
    }
    else if (events->type == K_POLL_TYPE_FIFO_DATA_AVAILABLE)
    {
        int tick_cnt = 0, i;
        struct k_poll_event *ev;

        while (tick_cnt <= timeout.ticks)
        {
            ev = events;
            for (i = 0; i < num_events; i++, ev++)
            {
                if (!k_fifo_is_empty(ev->fifo))
                {
                    ev->state = K_POLL_STATE_FIFO_DATA_AVAILABLE;
                    break;
                }
            }
            if (i < num_events)
                break;
            rt_thread_delay(RT_THREAD_TICK_DEFAULT);
            tick_cnt += RT_THREAD_TICK_DEFAULT;
        }
        if (tick_cnt >= timeout.ticks && timeout.ticks)
            r = - ETIME;
    }
    return r;
}

void k_poll_signal_reset(struct k_poll_signal *sig)
{
    sig->flag = RT_WQ_FLAG_CLEAN;
}

void k_fifo_cancel_wait(struct k_fifo *queue)
{
    // TODO
}

/**
 * @brief Initialize a poll signal object.
 *
 * Ready a poll signal object to be signaled via k_poll_signal_raise().
 *
 * @param sig A poll signal.
 */

__syscall void k_poll_signal_init(struct k_poll_signal *sig)
{
    rt_wqueue_init(sig);
}


int k_work_cancel_delayable(struct k_work_delayable *dwork)
{
    struct rt_work *work = (struct rt_work *)dwork;

    if (work->flags)
        return rt_work_cancel(work);
    else
        return 0;

}

void k_thread_start(rt_thread_t thread)
{
    if (thread->type == 0)
    {
        rt_thread_init(thread, thread->name, thread->entry,
                       thread->parameter, thread->stack_addr, thread->stack_size, thread->init_priority, 20);
    }
    if ((thread->stat & RT_THREAD_STAT_MASK) == RT_THREAD_INIT)
        rt_thread_startup(thread);
}

int k_thread_join(struct k_thread *thread, k_timeout_t timeout)
{
    int t = 0;
    int r = 0;

    if (thread == NULL)
        return 0;

    /* Lock the scheduler */
    rt_enter_critical();
    while (t < timeout.ticks)
    {
        /* If thread already exited, return immediately */
        if (thread->stat & RT_THREAD_CLOSE)
        {
            break;;
        }
        t += RT_THREAD_TICK_DEFAULT;
        rt_exit_critical();
        rt_thread_delay(RT_THREAD_TICK_DEFAULT);
        rt_enter_critical();
    }
    if (t >= timeout.ticks)
    {
        rt_thread_detach(thread);
        r = -ETIMEDOUT;
    }
    rt_exit_critical();

    return r;
}

void sibles_set_trc_cfg(sibles_trc_cfg_t cfg_mode, uint32_t mask_ext)
{
    // Dummy for compile
}



#if defined(BSP_USING_SPI_NAND) && defined(RT_USING_DFS)
#include "dfs_file.h"
#include "dfs_posix.h"
#include "drv_flash.h"
#define NAND_MTD_NAME    "root"
int mnt_init(void)
{
    //TODO: how to get base address
    register_nand_device(FS_REGION_START_ADDR & (0xFC000000), FS_REGION_START_ADDR - (FS_REGION_START_ADDR & (0xFC000000)), FS_REGION_SIZE, NAND_MTD_NAME);
    if (dfs_mount(NAND_MTD_NAME, "/", "elm", 0, 0) == 0) // fs exist
    {
        rt_kprintf("mount fs on flash to root success\n");
    }
    else
    {
        // auto mkfs, remove it if you want to mkfs manual
        rt_kprintf("mount fs on flash to root fail\n");
        if (dfs_mkfs("elm", NAND_MTD_NAME) == 0)
        {
            rt_kprintf("make elm fs on flash sucess, mount again\n");
            if (dfs_mount(NAND_MTD_NAME, "/", "elm", 0, 0) == 0)
                rt_kprintf("mount fs on flash success\n");
            else
                rt_kprintf("mount to fs on flash fail\n");
        }
        else
            rt_kprintf("dfs_mkfs elm flash fail\n");
    }
    return RT_EOK;
}
INIT_ENV_EXPORT(mnt_init);
#endif

static struct rt_semaphore zbt_rx_sem;
static rt_device_t rt_console;

rt_err_t (*old_rx_ind)(rt_device_t dev, rt_size_t size);

static rt_err_t console_rx_ind(rt_device_t dev, rt_size_t size)
{
    RT_ASSERT(rt_console != RT_NULL);

    /* release semaphore to let finsh thread rx data */
    rt_sem_release(&zbt_rx_sem);

    return RT_EOK;
}

int console_init(void)
{
    rt_sem_init(&zbt_rx_sem, "zbt_con", 0, RT_IPC_FLAG_FIFO);
    rt_console = rt_console_get_device();
    RT_ASSERT(rt_console != RT_NULL);

    old_rx_ind = rt_console->rx_indicate;
    rt_device_set_rx_indicate(rt_console, console_rx_ind);

    return 0;
}

int console_getchar(void)
{
    char ch = 0;

    while (rt_device_read(rt_console, -1, &ch, 1) != 1)
        rt_sem_take(&zbt_rx_sem, RT_WAITING_FOREVER);

    return (int)ch;
}

void console_done(void)
{
    rt_device_set_rx_indicate(rt_console, old_rx_ind);
}

void sys_reboot(int type)
{
    HAL_PMU_Reboot();
}


/**
 * @brief Fill the destination buffer with random data values that should
 * pass general randomness tests.
 *
 * @note The random values returned are not considered cryptographically
 * secure random number values.
 *
 * @param [out] dst destination buffer to fill with random data.
 * @param len size of the destination buffer.
 *
 */
void sys_rand_get(void *dst, size_t len)
{
    RNG_HandleTypeDef hrng;
    uint32_t val, seed;
    uint8_t *p_dst = (uint8_t *)dst;

    memset(&hrng, 0, sizeof(hrng));
    hrng.Instance = hwp_trng;
    HAL_RNG_Init(&hrng);

    HAL_RNG_GenerateRandomSeed(&hrng, &seed);
    for (int i = 0; i < len; i += sizeof(val))
    {
        HAL_RNG_GenerateRandomNumber(&hrng, &val);
        memcpy(p_dst + i, &val, len - i > sizeof(val) ? sizeof(val) : len - i);
    }
}

/**
 * @brief Fill the destination buffer with cryptographically secure
 * random data values.
 *
 * @note If the random values requested do not need to be cryptographically
 * secure then use sys_rand_get() instead.
 *
 * @param [out] dst destination buffer to fill.
 * @param len size of the destination buffer.
 *
 * @return 0 if success, -EIO if entropy reseed error
 *
 */
int sys_csrand_get(void *dst, size_t len)
{
    sys_rand_get(dst, len);
    return 0;
}

/********************************** Shell **************************************/
#include "zephyr/shell/shell.h"

static char rt_log_buf[RT_CONSOLEBUF_SIZE];
void shell_fprintf_normal(const struct shell *sh, const char *fmt, ...)
{
    va_list args;
    extern void rt_kputs(const char *str);

    va_start(args, fmt);
    rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    rt_kputs(rt_log_buf);
    va_end(args);
}


void shell_fprintf_error(const struct shell *sh, const char *fmt, ...)
{
    va_list args;
    extern void rt_kputs(const char *str);

    rt_kputs("E:");
    va_start(args, fmt);
    rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    rt_kputs(rt_log_buf);
    va_end(args);
}

void shell_fprintf_warn(const struct shell *sh, const char *fmt, ...)
{
    va_list args;
    extern void rt_kputs(const char *str);

    rt_kputs("W:");
    va_start(args, fmt);
    rt_vsnprintf(rt_log_buf, sizeof(rt_log_buf) - 1, fmt, args);
    rt_kputs(rt_log_buf);
    va_end(args);
}


#ifdef RT_USING_FINSH

#if 0
static cmd_function_t zbt_get_cmd(char *cmd, int size)
{
    struct finsh_syscall *index;
    cmd_function_t cmd_func = RT_NULL;

    for (index = _syscall_table_begin;
            index < _syscall_table_end;
            FINSH_NEXT_SYSCALL(index))
    {
        if (strncmp(index->name, "__cmd_", 6) != 0) continue;

        if (strncmp(&index->name[6], cmd, size) == 0 &&
                index->name[6 + size] == '\0')
        {
            cmd_func = (cmd_function_t)index->func;
            break;
        }
    }

    return cmd_func;
}

#endif

#ifdef SHELL
void shell_help(const struct shell *sh)
{
}

void shell_hexdump(const struct shell *sh, const uint8_t *data, size_t len)
{
    HAL_DBG_print_data(data, 0, len);
}

extern const struct shell_static_entry *z_shell_get_last_command(
    const struct shell_static_entry *entry,
    size_t argc,
    const char *argv[],
    size_t *match_arg,
    struct shell_static_entry *dloc,
    bool only_static);

void zbt(int argc, char **argv)
{
    const struct shell_static_entry *r = NULL;
    struct shell_static_entry dloc;

    int i;

    r = z_shell_get_last_command(NULL, argc - 1, (const char **) & (argv[1]), &i, &dloc, 0);
    if (r != NULL)
    {
        r->handler(NULL, argc - i, (char **) & (argv[i]));
    }
}
MSH_CMD_EXPORT(zbt, Zephyr shell command)
#endif
#endif

