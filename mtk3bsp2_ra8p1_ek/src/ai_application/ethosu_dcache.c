// #include "common_data.h"
// 
// void ethosu_flush_dcache(const uint64_t *base_addr, const size_t *base_addr_size, int num_base_addr)
// {
// #if (BSP_CFG_DCACHE_ENABLED == 1)
//     if(num_base_addr > 0 && base_addr != NULL) {
//         SCB_CleanDCache_by_Addr((uint32_t *)base_addr[0], (int32_t)base_addr_size[0]);
//     } else {
//         SCB_CleanDCache();
//     }
// #endif
// }
// 
// void ethosu_invalidate_dcache(const uint64_t *base_addr, const size_t *base_addr_size, int num_base_addr)
// {
// #if (BSP_CFG_DCACHE_ENABLED == 1)
//     if(num_base_addr > 0 && base_addr != NULL) {
//         SCB_InvalidateDCache_by_Addr((uint32_t *)base_addr[0], (int32_t)base_addr_size[0]);
//     } else {
//         SCB_InvalidateDCache();
//     }
// #endif
// }
