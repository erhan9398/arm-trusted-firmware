/*
 * Copyright 2020 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <clk/clk.h>
#include <common/debug.h>
#include <errno.h>
#include <libfdt.h>
#include <memory_pool.h>
#include <s32g_dt.h>

#define MAX_NUM_CLK 100
#define MAX_NUM_DRV 20

static struct clk_driver drivers[MAX_NUM_DRV];
static struct memory_pool drv_pool = INIT_MEM_POOL(drivers);

struct clk_driver *allocate_clk_driver(void)
{
	return alloc_mem_pool_elem(&drv_pool);
}

void set_clk_driver_name(struct clk_driver *drv, const char *name)
{
	size_t max_offset = sizeof(drv->name) - 1;
	size_t len = strnlen(name, max_offset);

	drv->name[len] = '\0';
	memcpy(drv->name, name, len);
}

struct clk *allocate_clk(void)
{
	static struct clk clks[MAX_NUM_CLK];
	static struct memory_pool clk_pool = INIT_MEM_POOL(clks);

	return alloc_mem_pool_elem(&clk_pool);
}

struct clk_driver *get_clk_driver(uint32_t phandle)
{
	size_t i;

	if (!phandle)
		return NULL;

	for (i = 0; i < drv_pool.fill_level; i++) {
		if (drivers[i].phandle == phandle)
			return &drivers[i];
	}

	return NULL;
}

struct clk_driver *get_clk_driver_by_name(const char *name)
{
	size_t i;
	size_t len = strnlen(name, CLK_DRV_NAME_SIZE);

	if (!name)
		return NULL;

	if (len + 1 < CLK_DRV_NAME_SIZE)
		len++;

	for (i = 0; i < drv_pool.fill_level; i++) {
		if (!memcmp(name, drivers[i].name, len))
			return &drivers[i];
	}

	return NULL;
}

void dt_clk_init(void)
{
	int ret;
	void *fdt;

	if (dt_open_and_check() < 0) {
		ERROR("Invalid DTB\n");
		return;
	}

	if (fdt_get_address(&fdt) == 0) {
		ERROR("Failed to get FDT address\n");
		return;
	}

	ret = dt_init_fixed_clk(fdt);
	if (ret) {
		ERROR("Failed to initialize fixed clocks\n");
		return;
	}

	ret = dt_init_plat_clk(fdt);
	if (ret) {
		ERROR("Failed to initialize platform clocks\n");
		return;
	}
}

int get_clk(uint32_t drv_id, uint32_t clk_id, struct clk *clock)
{
	struct clk_driver *clk_drv;

	clk_drv = get_clk_driver(drv_id);
	if (!clk_drv) {
		ERROR("Invalid clock driver ID: %d\n", drv_id);
		return -EIO;
	}

	clock->drv = clk_drv;
	clock->id = clk_id;

	if (clk_drv->ops->request)
		return clk_drv->ops->request(clk_id, clock);

	return 0;
}


static int process_parents_prop(int *parent_index, size_t nparents,
				const fdt32_t *parents, struct clk *clk,
				struct clk *parent_clk)
{
	uint32_t parent_drv_id, parent_clk_id;
	int ret;

	if (!parents)
		return 0;

	if (*parent_index >= nparents)
		return 0;

	parent_drv_id = fdt32_to_cpu(parents[*parent_index]);
	if (!parent_drv_id) {
		(*parent_index)++;
	} else {
		parent_clk_id = fdt32_to_cpu(parents[*parent_index + 1]);
		(*parent_index) += 2;

		ret = get_clk(parent_drv_id, parent_clk_id, parent_clk);
		if (ret) {
			ERROR("Unidentified clock id: %d\n", parent_clk_id);
			return -EIO;
		}

		if (clk->drv->ops && clk->drv->ops->set_parent) {
			ret = clk->drv->ops->set_parent(clk, parent_clk);
			if (ret) {
				ERROR("Failed to set parent of %d clk\n",
						clk->id);
			}
			return ret;
		}
	}

	return 0;
}

int dt_clk_apply_defaults(void *fdt, int node)
{
	const fdt32_t *clocks, *parents, *rates;
	int nclocks_size, nparents_size, nrates_size;
	int i, ret, index, parent_index;
	uint32_t clk_drv_id, clk_id, freq;
	struct clk clk, parent_clk;
	size_t nparents;

	clocks = fdt_getprop(fdt, node, "assigned-clocks", &nclocks_size);
	/* No clock settings to be applied */
	if (!clocks)
		return 0;

	parents = fdt_getprop(fdt, node, "assigned-clock-parents",
			      &nparents_size);

	rates = fdt_getprop(fdt, node, "assigned-clock-rates", &nrates_size);

	nparents = nparents_size / sizeof(uint32_t);

	parent_index = 0;
	for (i = 0; i < nclocks_size / FDT_CLOCK_CELL_SIZE; i++) {
		index = i * 2;

		clk_drv_id = fdt32_to_cpu(clocks[index]);
		clk_id = fdt32_to_cpu(clocks[index + 1]);
		if (!clk_drv_id) {
			ERROR("Invalid clock driver id: 0.\n");
			return -EIO;
		}

		ret = get_clk(clk_drv_id, clk_id, &clk);
		if (ret)
			return -EIO;

		ret = process_parents_prop(&parent_index, nparents,
					   parents, &clk, &parent_clk);
		if (ret) {
			ERROR("Failed to process parents field for '%s' node\n",
			      fdt_get_name(fdt, node, NULL));
			return -EINVAL;
		}

		if (!rates)
			continue;

		if (i >= nrates_size / sizeof(uint32_t)) {
			ERROR("Failed to process rates field for '%s' node\n",
			      fdt_get_name(fdt, node, NULL));
			return -EINVAL;

		}

		freq = fdt32_to_cpu(rates[i]);

		if (freq && clk.drv->ops && clk.drv->ops->set_rate) {
			ret = clk.drv->ops->set_rate(&clk, freq);
			if (!ret) {
				ERROR("Failed to set rate of %d clk\n",
						clk.id);
				return ret;
			}
		}

	}

	return 0;
}
