/*
 * Copyright 2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <assert.h>
#include <errno.h>
#include <arm/css/scmi/scmi_logger_private.h>
#include <drivers/nxp/s32/stm/s32_stm.h>
#include <s32_scmi_metadata.h>
#include <s32_platform_def.h>

/**
 * S32CC platform specific information
 * to be logged:
 *  - timestamps from the metadata memory region
 */
struct scmi_plat_data {
	uint32_t timestamps[TS_COUNT];
};

/**
 * A log entry consists of both general
 * and platform specific data
 */
struct s32_log_entry {
	struct scmi_log_entry base;
	struct scmi_plat_data plat_data;
};

static struct s32_stm timer;
static struct s32_log_entry scmi_log[SCMI_LOG_MAX_LEN];

static size_t get_metadata_size(void)
{
	return S32_SCP_SCMI_META_MEM_SIZE;
}

static struct scmi_log_entry *s32_get_log_entry(unsigned int index)
{
	assert(index < SCMI_LOG_MAX_LEN);
	return (struct scmi_log_entry *)&scmi_log[index];
}

static void s32_scmi_log_req_data(struct scmi_log_entry *entry, uintptr_t md_addr)
{
	struct s32_log_entry *s32_entry = (struct s32_log_entry *)entry;
	struct s32_scmi_metadata *md = (struct s32_scmi_metadata *)md_addr;
	uint32_t timestamp = s32_stm_get_count(&timer);

	if (!s32_entry || !md)
		return;

	md->timestamps[TS_AGENT_REQ_TX] = timestamp;
	s32_entry->plat_data.timestamps[TS_AGENT_REQ_TX] = timestamp;
}

static void s32_scmi_log_rsp_data(struct scmi_log_entry *entry, uintptr_t md_addr)
{
	struct s32_log_entry *s32_entry = (struct s32_log_entry *)entry;
	struct s32_scmi_metadata *md = (struct s32_scmi_metadata *)md_addr;
	uint32_t timestamp = s32_stm_get_count(&timer);

	if (!s32_entry || !md)
		return;

	md->timestamps[TS_AGENT_RSP_RX] = timestamp;
	s32_entry->plat_data.timestamps[TS_PLAT_REQ_RX] = md->timestamps[TS_PLAT_REQ_RX];
	s32_entry->plat_data.timestamps[TS_PLAT_RSP_TX] = md->timestamps[TS_PLAT_RSP_TX];
	s32_entry->plat_data.timestamps[TS_AGENT_RSP_RX] = timestamp;
}

static void s32_scmi_log_notif_data(struct scmi_log_entry *entry, uintptr_t md_addr)
{
	struct s32_log_entry *s32_entry = (struct s32_log_entry *)entry;
	struct s32_scmi_metadata *md = (struct s32_scmi_metadata *)md_addr;
	uint32_t timestamp = s32_stm_get_count(&timer);

	if (!s32_entry || !md)
		return;

	md->timestamps[TS_AGENT_NOTIF_RX] = timestamp;
	s32_entry->plat_data.timestamps[TS_PLAT_NOTIF_TX] = md->timestamps[TS_PLAT_NOTIF_TX];
	s32_entry->plat_data.timestamps[TS_AGENT_NOTIF_RX] = timestamp;
}

static void s32_scmi_log_notif_ack(struct scmi_log_entry *entry, uintptr_t md_addr)
{
	struct s32_log_entry *s32_entry = (struct s32_log_entry *)entry;
	struct s32_scmi_metadata *md = (struct s32_scmi_metadata *)md_addr;
	uint32_t timestamp = s32_stm_get_count(&timer);

	if (!s32_entry || !md)
		return;

	md->timestamps[TS_AGENT_ACK_RX] = timestamp;
	s32_entry->plat_data.timestamps[TS_AGENT_ACK_RX] = timestamp;
	/* last entry not used for notifications */
	s32_entry->plat_data.timestamps[TS_NOTIF_COUNT] = 0;
}

int log_scmi_plat_init(struct scmi_logger *logger)
{
	int ret = 0;

	assert(sizeof(struct s32_scmi_metadata) <= S32_SCP_CH_META_SIZE);
	assert(ARRAY_SIZE(scmi_log) >= PLATFORM_CORE_COUNT);

	if (!logger)
		return -EINVAL;

	logger->get_entry = s32_get_log_entry;
	logger->log_req_data = s32_scmi_log_req_data;
	logger->log_rsp_data = s32_scmi_log_rsp_data;
	logger->log_notif_data = s32_scmi_log_notif_data;
	logger->log_notif_ack = s32_scmi_log_notif_ack;

	ret = s32_stm_init(&timer);
	if (ret) {
		ERROR("Failed to initialize STM timer.\n");
		return ret;
	}

	if (!s32_stm_is_enabled(&timer)) {
		/**
		 * If the timer was not yet enabled,
		 * neither was the metadata initialised.
		 */
		memset((void *)S32_SCP_SCMI_META_MEM, 0, get_metadata_size());
		s32_stm_enable(&timer, true);
	}

	return 0;
}

