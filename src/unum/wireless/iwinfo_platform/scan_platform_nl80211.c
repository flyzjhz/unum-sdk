// (c) 2020 minim.co
// unum platform code for collecting wireless neighborhood scanlist info

#include "unum.h"

// Include declarations for platform driver specific code
// we share locally.
#include "wireless_platform.h"
#include "../wireless_nl80211.h"


/* Temporary, log to console from here */
//#undef LOG_DST
//#undef LOG_DBG_DST
//#define LOG_DST LOG_DST_CONSOLE
//#define LOG_DBG_DST LOG_DST_CONSOLE


// Buffer for the scanlist, once allocated it is never freed.
// The same buffer is used for all radios (we process one radio at a time).
static char *scanlist_buf = NULL;
static const char *ht_chanwidth[2] = {
    "20 MHz",
    "any",
};

static const char *vht_chanwidth[] = {
    "20 or 40 MHz",
    "80 MHz",
    "80+80 MHz",
    "160 MHz",
};
#ifdef FEATURE_MAC80211_LIBNL
char*
wt_platform_iwinfo_get_vap(char *phyname)
{
    int phyidx = wt_iwinfo_get_phy_num(phyname);
    int idx = -1;
    char *ifname;

    if(phyidx < 0) {
        log("%s: invalid phy name <%s>\n", __func__, phyname);
        return NULL;
    }
    if ((idx = wt_iwinfo_get_if_num_for_phy(phyidx, idx)) >= 0)
    {
        ifname = wt_iwinfo_get_ifname(idx);
    } else {
        log("%s: No Interfaces <%s>\n", __func__, phyname);
        return NULL;
    }
    return ifname;
}
#endif

int wt_platform_iwinfo_get_scanlist(char *ifname,
                                WT_JSON_TPL_SCAN_RADIO_t *rscan)
{
    int len;

    if (scanlist_buf == NULL) {
        scanlist_buf = (char *)malloc(SCANLIST_BUF_SIZE);
    }
    if (scanlist_buf == NULL) {
        log("%s: Error while allocating buffer for scanlist\n", __func__);
        return -1;
    }
    memset(scanlist_buf, 0, SCANLIST_BUF_SIZE);
    len = wt_nl80211_get_scan(ifname, scanlist_buf);
    if(len < 0)
    {
        // scan might not work due to DFS channel, log error in debug mode only
        // BTW, the upper layer still going to log an error
        log_dbg("%s: iwinfo scan error on <%s>, returned %d\n",
                __func__, ifname, len);
        return -2;
    }

    // Store the number of scan entries
    rscan->scan_entries = len;

    return 0;
}

// Extract the scan entry info
// Returns: 0 - if successful (all required info captured),
//          negative - error, positive - skip (no error)
int wt_platform_tpl_fill_scan_entry_info(WT_JSON_TPL_SCAN_RADIO_t *rscan,
                                WT_JSON_TPL_SCAN_ENTRY_t *entry, int ii)
{
    static char ht_width[16];
    static char vht_width[16];
    static char encryption[ENC_BUF_SIZE];

    static JSON_OBJ_TPL_t extras_obj = { // stats must be first
      // With nldrivers quality is 110 + RSSI and max quality is 70
      // Hence Not including them here as RSSI is anyhow included
      { "ht_width",   { .type = JSON_VAL_STR, {.s = ht_width  }}},
      { "vht_width",   { .type = JSON_VAL_STR, {.s = vht_width  }}},
      { "encryption",  { .type = JSON_VAL_STR, .s = encryption }},
      { NULL }
    };
    // We must have the scan data if we are here
    if(scanlist_buf == NULL) {
        return -1;
    }

    // Check if the index is out of range (it should never happen).
    if(ii < 0 || ii >= rscan->scan_entries) {
        return -2;
    }

    // Calculate pointer to the requested entry
    struct nl80211_scanlist_entry *e = (void *)scanlist_buf;
    e += ii;

    // Make sure we are not going beyond the buffer when accessing the entry
    if((void *)e + sizeof(struct nl80211_scanlist_entry) >
       (void *)scanlist_buf + SCANLIST_BUF_SIZE)
    {
        return -3;
    }

    // Capture main channel
    entry->chan = e->channel;

    // BSSID
    snprintf(entry->bssid, sizeof(entry->bssid), MAC_PRINTF_FMT_TPL,
             MAC_PRINTF_ARG_TPL(e->bssid));

    // SSID. Due to the limitation of the data we will not catch
    // SSIDs w/ 0 in the middle
    if(wt_mk_hex_ssid(entry->ssid, e->ssid, strlen(e->ssid)) != 0)
    {
        log("%s: invalid SSID for %s entry %d\n", __func__, rscan->name, ii);
        return -4;
    }
    // RSSI
    entry->rssi = e->signal;

    // Copy Chanwidth as a string
    // VHT involves strings like 80 + 80
    // So using strings instead of integers
    memset(ht_width, 0, sizeof(ht_width));
    memset(vht_width, 0, sizeof(ht_width));

    if (e->ht_chwidth == 0 || e->ht_chwidth == 1) {
        strcpy(ht_width, ht_chanwidth[e->ht_chwidth]);
    } else {
        strcpy(ht_width, "unknown");
    }

    if (e->vht_chwidth >= 0 && 
               e->vht_chwidth < (sizeof(vht_chanwidth)/sizeof(vht_chanwidth[0]))) {
        strcpy(vht_width, vht_chanwidth[e->vht_chwidth]);
    } else {
        strcpy(vht_width, "unknown");
    }

    // Encryption
    memset(encryption, 0, ENC_BUF_SIZE);
    strncpy(encryption, e->enc_buf, ENC_BUF_SIZE - 1);

    // Set the extras object template ptr
    entry->extras = &(extras_obj[0]);

    return 0;
}

