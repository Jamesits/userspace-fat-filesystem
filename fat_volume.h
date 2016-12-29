#ifndef _FAT_VOLUME_H
#define _FAT_VOLUME_H

#include "fat_types.h"
#include "fat_file.h"
#include "list.h"
#include <sys/types.h>

enum fat_type {
	FAT_TYPE_FAT12 = 0,
	FAT_TYPE_FAT16,
	FAT_TYPE_FAT32,
};

struct fat_volume {

	/* FAT mapped into memory */
	void *fat_map;

	/* Number of data clusters */
	u32 num_data_clusters;

	/* Byte offset of the "second" cluster (first in the actual layout) */
	off_t data_start_offset;

	/* Flags passed to fat_mount() */
	int mount_flags;

	/* FAT12, FAT16, or FAT32 */
	enum fat_type type;

	/* Root directory */
	struct fat_file root;

	/* Least-recently-used list of directories with in-memory children, so
	 * we can free children that are no longer needed. */
	struct list_head lru_file_list;

	/* Number of `struct fat_file's that have been allocated. */
	size_t num_allocated_files;

	/* Maximum number of `struct fat_file's to allocate (soft limit only) */
	size_t max_allocated_files;

	/* Open file descriptor to the volume file or device */
	int fd;

	/* Standard boot sector info */
	char oem_name[8 + 1];

	/* Data from DOS 2.0 BIOS Parameter Block */
	u16 bytes_per_sector;
	u16 sector_order;
	u8  sectors_per_cluster;
	u8  sectors_per_cluster_order;
	u16 cluster_order;
	u16 reserved_sectors;
	u8  num_tables;
	u16 max_root_entries;
	u8  media_descriptor;

	u32 total_sectors;
	u32 sectors_per_fat; /* 16-bit in DOS 2.0 BPB, 32-bit in FAT32 EBPB */

	/* Data from DOS 3.0 BIOS Parameter Block */
	u16 sectors_per_track;
	u16 num_heads;
	u32 hidden_sectors;

	/* Data from FAT32 Extended BIOS parameter block */
	u16 drive_description;
	u16 version;
	u32 root_dir_start_cluster;
	u16 fs_info_sector;
	u16 alt_boot_sector;


	/* Data from non-FAT32 Extended BIOS parameter block */
	u8 physical_drive_num;
	u8 extended_boot_sig;
	u32 volume_id;
	char volume_label[11 + 1];
	char fs_type[8 + 1];
};

#define FAT_MOUNT_FLAG_READWRITE	0x1

extern struct fat_volume *
fat_mount(const char *volume, int mount_flags);

extern int
fat_unmount(struct fat_volume *vol);

static inline bool
fat_is_valid_cluster_number(const struct fat_volume *vol, u32 cluster)
{
	return cluster >= 2 && cluster < vol->num_data_clusters + 2;
}

extern u32
fat_next_cluster(struct fat_volume *vol, u32 cur_cluster);

static inline off_t
fat_data_cluster_offset(const struct fat_volume *vol, u32 cluster)
{
	return vol->data_start_offset +
		((off_t)(cluster - 2) << vol->cluster_order);
}

#define FAT_CLUSTER_END_OF_CHAIN 0xFFFFFFFF

/* Extended BIOS Parameter Block (non-FAT-32 version) */
struct nonfat32_ebpb {
    u8 physical_drive_num;
    u8 reserved;
    u8 extended_boot_sig;
    le32 volume_id;
    char volume_label[11];
    char fs_type[8];
} __attribute__((packed));

/* Extended BIOS Parameter Block (FAT-32 version) */
struct fat32_ebpb {
    le32 sectors_per_fat;
    le16 drive_description;
    le16 version;
    le32 root_dir_start_cluster;
    le16 fs_info_sector;
    le16 alt_boot_sector;
    u8 reserved[12];
} __attribute__((packed));

/* On-disk format of the FAT boot sector, starting from the very first byte of
 * the device. */
struct fat_boot_sector_disk {
    /* Jump instruction for the bootloader (ignored) */
    u8 jump_insn[3];
    
    /* Standard boot sector info */
    char oem_name[8];
    
    /* DOS 2.0 BIOS Parameter Block (13 bytes) */
    le16 bytes_per_sector;
    u8   sectors_per_cluster;
    le16 reserved_sectors;
    u8   num_tables;
    le16 max_root_entries;
    le16 total_sectors;
    u8   media_descriptor;
    le16 sectors_per_fat;
    
    /* DOS 3.31 BIOS Parameter Block (12 bytes) */
    le16 sectors_per_track;
    le16 num_heads;
    le32 hidden_sectors;
    le32 total_sectors_32;
    
    /* Extended BIOS Parameter Block (different depending on whether the FAT
     * version is FAT32 or not) */
    union __attribute__((packed)) {
        struct __attribute__((packed)) {
            struct fat32_ebpb fat32_ebpb;
            struct nonfat32_ebpb nonfat32_ebpb;
        } fat32;
        struct nonfat32_ebpb nonfat32_ebpb;
    } ebpb;
} __attribute__((packed));


#endif /* _FAT_VOLUME_H */
