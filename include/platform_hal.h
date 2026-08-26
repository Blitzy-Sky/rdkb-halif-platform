/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
/**
 * 
 * @file platform_hal.h
 * @brief The platform_hal provides an interface to get/set platform control features.
 */

#ifndef __PLATFORM_HAL_H__
#define __PLATFORM_HAL_H__

/**
* @defgroup platform_hal Platform HAL
*
* The Platform HAL abstracts the board and device details that do not belong to any single
* subsystem: model, region and serial identity, firmware and bootloader naming, flash and
* DRAM sizing, front-panel LED indication, fan and thermal control, MACsec administration,
* DSCP traffic accounting and QoS rule programming. A vendor supplies the implementation
* behind this interface; the caller is RDK-B middleware.
*
* @par Runtime contract
* The obligations in this section are stated once here and hold for every function in
* @ref PLATFORM_HAL_APIS. Each function's own documentation adds only what is specific to
* it, so a caller should read this section first and the function afterwards.
*
* @li <b>Execution model.</b> Every function is synchronous: the outcome is complete when
*     the function returns, and the calling thread is blocked until it does. A function is
*     expected to finish within a period commensurate with the complexity of the operation.
*     Where a function can fail through the absence of a response, the implementation is
*     expected to bound its wait with a timeout. The initialization functions are expected
*     to block while the hardware is not yet ready. This interface does not publish a
*     numeric time bound for any individual function.
* @li <b>Asynchronous notification.</b> There is none. This interface declares no callback
*     type and no registration function, so a caller that needs to observe a change polls
*     the relevant getter rather than subscribing to an event.
* @li <b>Thread safety.</b> This interface is <b>not</b> thread safe. A caller that invokes
*     two Platform HAL functions concurrently, whether from one thread or several, must
*     provide its own serialisation. A vendor implementation is permitted to create internal
*     threads, in which case it is responsible for synchronising its own calls and events
*     and for cleaning those threads up. All functions are expected to be callable from more
*     than one process.
* @li <b>Memory model.</b> Unless a function states otherwise, the caller allocates and releases every buffer it passes in and
*     the buffer remains owned by the caller. This interface does not specify whether an implementation keeps a caller-supplied
*     pointer beyond the call that carried it, so a caller must not assume either behaviour: nothing stated here guarantees that the
*     implementation has dropped the pointer by the time the call returns, and a caller that needs that guarantee must obtain it from
*     the implementation it runs against. @c platform_hal_GetMemoryPaths is the one documented exception, where the HAL performs the
*     allocation. A vendor implementation may make further internal allocations, which it is responsible for releasing itself.
* @li <b>Error reporting.</b> Errors are reported synchronously through the return value.
*     System-level errors such as memory exhaustion are handled inside the HAL and are not
*     surfaced as separate codes, so the return value is the only failure indication a
*     caller receives.
* @li <b>Persistence.</b> This interface carries no persistence obligation. A caller that
*     needs a setting to survive a restart must persist that setting itself.
* @li <b>Power management.</b> The HAL takes no part in power management, and a power state
*     transition must not disturb its operation. @c platform_hal_SetLowPowerModeState and
*     @c PSM_STATE report and set a platform power-saving state as data-model values; they
*     do not make this HAL a participant in power management.
* @li <b>Build variability.</b> Two compile-time defines gate parts of this interface,
*     @c FEATURE_RDKB_LED_MANAGER and @c FEATURE_RDKB_THERMAL_MANAGER. Every declaration
*     that one of them gates names the define it requires. Nine of the functions declared
*     here are gated in this way; the remaining sixty-two are always present.
* @li <b>Constant definitions.</b> Every constant in @ref PLATFORM_HAL_TYPES is defined
*     inside an @c ifndef guard, so a translation unit that has already defined the same
*     name keeps its own definition.
*
* Derived from the declarations in this file and from the Platform HAL specification under
* @c docs/pages, sections "Component Runtime Execution Requirements" and "Non functional
* requirements". Where this file and that specification disagree, this file governs.
*
* @defgroup PLATFORM_HAL_TYPES  Platform HAL Data Types
* @ingroup  platform_hal
*
* @defgroup PLATFORM_HAL_APIS   Platform HAL  APIs
* @ingroup  platform_hal
*
**/


/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */


/**********************************************************************
               CONSTANT DEFINITIONS
**********************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

#ifndef CHAR
#define CHAR  char /*!< @brief Character type alias used for the string buffers in this interface. */
#endif

#ifndef UCHAR
#define UCHAR unsigned char /*!< @brief Unsigned character type alias, used for raw byte values. */
#endif

#ifndef BOOLEAN
#define BOOLEAN  unsigned char /*!< @brief Boolean type alias; carries @c TRUE or @c FALSE. */
#endif

#ifndef INT
#define INT   int /*!< @brief Signed integer type alias; the return type of most functions here. */
#endif

#ifndef UINT
#define UINT  unsigned int /*!< @brief Unsigned integer type alias, used for indices and counts. */
#endif

#ifndef ULONG
#define ULONG unsigned long /*!< @brief Unsigned long type alias, used for sizes, counts and timeouts. */
#endif

#ifndef UINT8_t
#define UINT8_t unsigned char /*!< @brief Fixed-width 8-bit unsigned integer type alias. */
#endif

#ifndef UINT16_t
#define UINT16_t unsigned short /*!< @brief Fixed-width 16-bit unsigned integer type alias. */
#endif

#ifndef UINT32_t
#define UINT32_t unsigned int /*!< @brief Fixed-width 32-bit unsigned integer type alias. */
#endif

#ifndef UINT64_t
#define UINT64_t unsigned long long /*!< @brief Fixed-width 64-bit unsigned integer type alias. */
#endif

#ifndef TRUE
#define TRUE     1 /*!< @brief Value of a @c BOOLEAN argument or output meaning true. */
#endif

#ifndef FALSE
#define FALSE    0 /*!< @brief Value of a @c BOOLEAN argument or output meaning false. */
#endif

#ifndef ENABLE
#define ENABLE   1 /*!< @brief Value denoting an enabled state. No declaration in this file refers to it. */
#endif

#ifndef RETURN_OK
#define RETURN_OK   0 /*!< @brief The operation completed successfully. */
#endif

#ifndef RETURN_ERR
/**
 * @brief The operation failed.
 *
 * This interface reports no further distinction between failure causes, because system-level
 * errors are handled inside the HAL rather than surfaced as separate codes.
 */
#define RETURN_ERR   -1
#endif

#ifndef RETURN_UNSUPPORTED
/**
 * @brief The platform does not implement the function that returned this code.
 *
 * Added to this interface in release 1.3.3 under RDKB-65287, together with the thermal manager
 * functions. It is returned only by those functions whose own documentation lists it, so a
 * caller must not expect it from any other function here. Unlike @c RETURN_ERR this outcome is
 * permanent for the platform, so a caller should stop polling rather than retry.
 */
#define RETURN_UNSUPPORTED   -2
#endif

//DHCPv6 Options
#define DHCPV6_OPT_3    3  /*!< @brief Identity Association for Non-temporary Addresses Option */
#define DHCPV6_OPT_5    5  /*!< @brief IA Address Option */
#define DHCPV6_OPT_15  15  /*!< @brief User Class Option */
#define DHCPV6_OPT_16  16  /*!< @brief Vendor Class Option */
#define DHCPV6_OPT_17  17  /*!< @brief OPTION_VENDOR_OPTS */
#define DHCPV6_OPT_20  20  /*!< @brief Reconfigure Accept Option */
#define DHCPV6_OPT_22  22  /*!< @brief OPTION_SIP_SERVER_A */
#define DHCPV6_OPT_23  23  /*!< @brief DNS Recursive Name Server option */
#define DHCPV6_OPT_24  24  /*!< @brief OPTION_DOMAIN_LIST */
#define DHCPV6_OPT_25  25  /*!< @brief Identity Association for Prefix Delegation Option */
#define DHCPV6_OPT_31  31  /*!< @brief OPTION_SNTP_SERVERS */
#define DHCPV6_OPT_64  64  /*!< @brief OPTION_AFTR_NAME */
#define DHCPV6_OPT_82  82  /*!< @brief OPTION_SOL_MAX_RT: Solicite Maximum Retry Time */
#define DHCPV6_OPT_83  83  /*!< @brief OPTION_INF_MAX_RT */
#define DHCPV6_OPT_95  95  /*!< @brief OPTION_SOL_MAX_RT: Solicite Maximum Retry Time */

//DHCPv4 Options
#define DHCPV4_OPT_2   2   /*!< @brief Time Offset */
#define DHCPV4_OPT_42  42  //!< NTP Server Addresses
#define DHCPV4_OPT_43  43  //!< Vendor Specific Information
#define DHCPV4_OPT_58  58  //!< DHCP Renewal (T1) Time
#define DHCPV4_OPT_59  59  //!< DHCP Rebinding (T2) Time
#define DHCPV4_OPT_60  60  //!< Class Identifier
#define DHCPV4_OPT_61  61  //!< Client Identifier
#define DHCPV4_OPT_100 100 //!< IEEE 1003.1 TZ String
#define DHCPV4_OPT_122 122 //!< CableLabs Client Configuration
#define DHCPV4_OPT_125 125 //!< Vendor-Identifying Vendor-Specific Information
#define DHCPV4_OPT_242 242 //!< Private Use
#define DHCPV4_OPT_243 243 //!< Private Use
#define DHCPV4_OPT_END 255 //!< DHCP Option End - used to check if option is valid

#define FW_NAME_MAX_LEN 64 /*!< @brief Buffer length, in bytes, of a firmware image name string. */
#define FW_STATE_MAX_LEN 64 /*!< @brief Buffer length, in bytes, of a firmware bank state string. */

/**********************************************************************
                ENUMERATION DEFINITIONS
**********************************************************************/
/**
 * @brief Represents the types of CPUs in an RDK system.
 *
 * This enumeration is used to distinguish between different CPU types 
 * within an RDK (Reference Design Kit) environment. It provides a mechanism 
 * to differentiate between the main host processor, any secondary or peer 
 * processors, and processors that are not supported or recognized within the 
 * RDK framework.
 */
typedef enum {
   HOST_CPU=0,          /*!< Represents the main host CPU, which typically handles
                         *   primary tasks and operations in the RDK environment. */

   PEER_CPU=1,          /*!< Represents a secondary or peer CPU. These CPUs might be
                         *   used for offloading specific tasks, handling parallel
                         *   processes, or managing other dedicated operations
                         *   alongside the main host CPU. */

   NOT_SUPPORTED_CPU=3  /*!< Denotes an unsupported or unrecognized CPU type. This value
                         *   can be used to identify and handle scenarios where a CPU type
                         *   is encountered that doesn't match known configurations in
                         *   the RDK environment. */
} 
RDK_CPUS; //!< Enum type indicating the CPU classifications within the RDK framework.

#ifndef PLAT_PROC_MEM_MAX_LEN
#define PLAT_PROC_MEM_MAX_LEN   40 /*!< @brief Maximum length in bytes of a memory path string, including its terminator. */
#endif

/**********************************************************************
                STRUCTURE DEFINITIONS
**********************************************************************/
/**
 * @brief Represents memory paths in a platform.
 *
 * Holds information about the memory paths used by a specific platform.
 * Provides paths for DRAM and multiple eMMC memories. These paths are generally
 * used for accessing or mounting memory storage locations in the system.
 *
 * @note Ensure that any path specified does not exceed the buffer size
 * limit defined by the `PLAT_PROC_MEM_MAX_LEN` macro.
 */
typedef struct _PLAT_PROC_MEM_INFO
{
    CHAR dramPath[PLAT_PROC_MEM_MAX_LEN];   /**< @brief Path to DRAM memory.
                                             *   Typically set to "/tmp".
                                             *   Ensure the path doesn't exceed
                                             *   a 40-byte buffer size. */

    CHAR emmcPath1[PLAT_PROC_MEM_MAX_LEN];  /**< @brief Path to the first eMMC memory.
                                             *   Possible values include "/nvram".
                                             *   Path should not exceed 40 bytes. */

    CHAR emmcPath2[PLAT_PROC_MEM_MAX_LEN];  /**< @brief Path to the second eMMC memory.
                                             *   "/nvram2" is a common value.
                                             *   Ensure path is under 40 bytes. */

    CHAR emmcPath3[PLAT_PROC_MEM_MAX_LEN];  /**< @brief Path to the third eMMC memory.
                                             *   This field is currently unused.
                                             *   Despite this, its value should
                                             *   not exceed 40 bytes. */
}
PLAT_PROC_MEM_INFO, *PPLAT_PROC_MEM_INFO;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**********************************************************************************
 *
 *  Subsystem level function prototypes
 *
**********************************************************************************/
/**
 * @brief Reports how far the device has progressed through its configuration sequence.
 *
 * Lets a caller decide whether configuration-dependent work may proceed yet, by returning the
 * platform's current configuration state as a short text token.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the status string as text.
 *                      Possible values are 'WaitForImplement', 'In Progress' and 'Complete'. The
 *                      buffer must be at least 128 bytes, the recommended maximum length of the
 *                      returned string. This interface states neither how many bytes the
 *                      implementation writes nor whether it writes a terminator, so a caller must
 *                      bound every read to the 128 bytes it supplied and, where a terminated string
 *                      is required, must do so itself: zero-filling alone leaves no terminator if
 *                      the implementation writes the whole buffer, so allocate one byte beyond the
 *                      size this interface requires and zero that byte before the call. This
 *                      interface does not specify whether the implementation keeps the pointer
 *                      after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 128 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the buffer holds one of the three listed tokens. On failure this interface does
 *       not define the buffer's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the operation is successful.
 * @retval RETURN_ERR if any error is detected during the operation. The caller should treat the
 *                    configuration status as unknown, discard the output buffer, and retry or
 *                    report the failure; no more specific cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_GetDeviceConfigStatus(CHAR *pValue);

/**
 * @brief Reports whether Telnet access is currently enabled.
 *
 * @param[out] pFlag - Caller-allocated, caller-owned @c BOOLEAN receiving the current setting.
 *                     Set to @c TRUE when Telnet is enabled and @c FALSE when it is disabled. This interface does
 *                     not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pFlag must point to a caller-allocated buffer of at least one BOOLEAN. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the referenced @c BOOLEAN holds @c TRUE or @c FALSE. On failure this interface
 *       does not define its value.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the setting as unknown and
 *                    must not infer that Telnet is disabled from the failure.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_SetTelnetEnable
 */
INT platform_hal_GetTelnetEnable(BOOLEAN *pFlag);

/**
 * @brief Enables or disables Telnet access.
 *
 * @param[in] Flag - Desired setting, passed by value. Pass @c TRUE to enable Telnet and @c FALSE
 *                   to disable it.
 *
 * @pre None established by this interface. The valid values of @c Flag are @c TRUE and @c FALSE; this
 *      interface does not define the behaviour for any other value.
 * @post On success Telnet access is in the requested state. On failure this interface does not
 *       define whether the previous setting was retained or partially changed, so a caller that
 *       needs certainty should read the setting back with @c platform_hal_GetTelnetEnable.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the setting as indeterminate
 *                    and read it back before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetTelnetEnable
 */
INT platform_hal_SetTelnetEnable(BOOLEAN Flag);

/**
 * @brief Reports whether SSH access is currently enabled.
 *
 * @param[out] pFlag - Caller-allocated, caller-owned @c BOOLEAN receiving the current setting.
 *                     Set to @c TRUE when SSH is enabled and @c FALSE when it is disabled. This interface does not
 *                     specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pFlag must point to a caller-allocated buffer of at least one BOOLEAN. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the referenced @c BOOLEAN holds @c TRUE or @c FALSE. On failure this interface
 *       does not define its value.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the setting as unknown and
 *                    must not infer that SSH is disabled from the failure.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_SetSSHEnable
 */
INT platform_hal_GetSSHEnable(BOOLEAN *pFlag);

/**
 * @brief Enables or disables SSH access.
 *
 * @param[in] Flag - Desired setting, passed by value. Pass @c TRUE to enable SSH and @c FALSE to
 *                   disable it.
 *
 * @pre None established by this interface. The valid values of @c Flag are @c TRUE and @c FALSE; this
 *      interface does not define the behaviour for any other value.
 * @post On success SSH access is in the requested state. On failure this interface does not define
 *       whether the previous setting was retained, so a caller that needs certainty should read the
 *       setting back with @c platform_hal_GetSSHEnable.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the setting as indeterminate
 *                    and read it back before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetSSHEnable
 */
INT platform_hal_SetSSHEnable(BOOLEAN Flag);

/**
 * @brief Reports which interface scope SNMP is currently enabled for.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the scope token as text. The
 *                      buffer must be at least 32 bytes, the recommended maximum length of the returned
 *                      string. Possible values are 'rgWan' (SNMP enabled for the WAN interface),
 *                      'rgDualIp' (enabled for a dual-IP configuration) and 'rgLanIp' (enabled for the
 *                      LAN IP configuration). This interface states neither how many bytes the
 *                      implementation writes nor whether it writes a terminator, so a caller must bound
 *                      every read to the 32 bytes it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator if the
 *                      implementation writes the whole buffer, so allocate one byte beyond the size this
 *                      interface requires and zero that byte before the call. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 32 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the buffer holds one of the three listed tokens. On failure this interface does
 *       not define the buffer's contents.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the operation is successful.
 * @retval RETURN_ERR if any error is detected during the operation. The caller should treat the
 *                    scope as unknown and discard the output buffer.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_SetSNMPEnable
 */
INT platform_hal_GetSNMPEnable(CHAR *pValue);

/**
 * @brief Selects the interface scope SNMP is enabled for.
 *
 * @param[in] pValue - Caller-allocated, caller-owned buffer of at least 32 bytes. The caller must
 *                     supply the scope token to apply as a zero-terminated string; termination is the
 *                     caller's obligation on the value it passes in. Accepted values are 'rgWan' (enable
 *                     SNMP for the WAN interface), 'rgDualIp' (enable for a dual-IP configuration) and
 *                     'rgLanIp' (enable for the LAN IP configuration). The buffer is read during the call;
 *                     this interface does not specify whether the implementation keeps the pointer after
 *                     the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 32 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected. This interface does not define the
 *      behaviour for a token outside the accepted set.
 * @post On success SNMP is enabled for the requested scope. On failure this interface does not
 *       define whether the previous scope was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the operation is successful.
 * @retval RETURN_ERR if any error is detected during the operation. The caller should read the
 *                    scope back with @c platform_hal_GetSNMPEnable before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetSNMPEnable
 */
INT platform_hal_SetSNMPEnable(CHAR *pValue);

/**
 * @brief Reports the configured web user-interface inactivity timeout.
 *
 * @param[out] pValue - Caller-allocated, caller-owned @c ULONG receiving the timeout in seconds. This call documents the value as lying
 *                      between 30 and 86400 seconds with a default of 840, which is narrower than the domain @c platform_hal_SetWebUITimeout
 *                      accepts: that call also accepts 0. This interface does not state what a read of 0 means, nor whether a read can return it,
 *                      so a caller must treat 0, and any other value outside 30 to 86400, as unspecified. This interface does not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least one ULONG. This interface does not define the behaviour for a
 *      null or undersized buffer, so a caller must not rely on either being detected.
 * @post On success the referenced @c ULONG holds the configured timeout; because the setter's accepted domain is wider than the range
 *       documented above, this interface does not guarantee that the value falls inside that range. On failure it does not define the value at all.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the timeout as unknown
 *                    rather than substituting the default.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_SetWebUITimeout
 */
INT platform_hal_GetWebUITimeout(ULONG *pValue);

/**
 * @brief Sets the web user-interface inactivity timeout.
 *
 * @param[in] value - Timeout to apply, in seconds, passed by value. Accepted values are 0, and any value from 30 to 86400 inclusive. A value of 0
 *                    may select a distinct mode such as no timeout; this interface does not define which. @c platform_hal_GetWebUITimeout documents a
 *                    narrower read range of 30 to 86400, so after writing 0 a caller cannot rely on the read-back falling inside the range that call documents.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a value
 *      between 1 and 29 or above 86400.
 * @post On success the timeout is in effect for subsequent sessions. On failure this interface does
 *       not define whether the previous timeout was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should read the value back with
 *                    @c platform_hal_GetWebUITimeout before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetWebUITimeout
 */
INT platform_hal_SetWebUITimeout(ULONG value);

/**
 * @brief Reads the web access level granted to a user on an interface.
 *
 * @param[in] userIndex - User index identifying whose access level is requested. The
 *                        representable range is -2^31 to (2^31)-1; the combinations this interface
 *                        gives meaning to are tabulated on @c platform_hal_SetWebAccessLevel.
 * @param[in] ifIndex - Interface index identifying which interface the level applies to. The
 *                      representable range is -2^31 to (2^31)-1.
 * @param[out] pValue - Caller-allocated, caller-owned @c ULONG receiving the
 *                      access level, in the range 0 to (2^31)-1. This interface does not specify
 *                      whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least one ULONG. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected. This interface does not define the
 *      behaviour for an unrecognised @c userIndex or @c ifIndex combination.
 * @post On success the referenced @c ULONG holds the access level. On failure this interface does
 *       not define its value.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the level as unknown and
 *                    must not fall back to a permissive default.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_SetWebAccessLevel
 *
 * @deprecated This function is scheduled for removal from this interface. New callers should not
 *             take a dependency on it.
 */
INT platform_hal_GetWebAccessLevel(INT userIndex, INT ifIndex, ULONG *pValue);

/**
 * @brief Sets the web access level granted to a user on an interface.
 *
 * @param[in] userIndex - User index identifying whose access level is being set. The representable
 *                        range is -2^31 to (2^31)-1; the meaningful combinations are tabulated
 *                        below.
 * @param[in] ifIndex - Interface index identifying which interface the level applies to. The
 *                      representable range is -2^31 to (2^31)-1.
 * @param[in] value - Access level to apply, passed by value. The valid range is 0 to (2^31)-1.
 *
 * <table>
 * <caption id="multi_row">Lan Level Vs Index table</caption>
 * <tr><th>Lan Level	<th>User Index	<th>Interface Index
 * <tr><td>HomeUser_Lan_Level <td>1 <td>1
 * <tr><td>HomeUser_RfCM_Level <td>1 <td>2
 * <tr><td>HomeUser_Mta_Level <td>1 <td>16
 * <tr><td>HomeUser_WanRG_Level <td>1 <td>40
 * <tr><td>CusAdmin_Lan_Level <td>5 <td>1
 * <tr><td>CusAdmin_RfCM_Level <td>5 <td>2
 * <tr><td>CusAdmin_Mta_Level <td>5 <td>16
 * <tr><td>CusAdmin_WanRG_Level <td>5 <td>40
 * <tr><td>AdvUser_Lan_Level <td>10 <td>1
 * <tr><td>AdvUser_RfCM_Level <td>10 <td>2
 * <tr><td>AdvUser_Mta_Level <td>10 <td>16
 * <tr><td>AdvUser_WanRG_Level <td>10 <td>40
 * </table>
 *
 * @pre None established by this interface. This interface does not define the behaviour for an index
 *      combination absent from the table above.
 * @post On success the requested access level is in effect. On failure this interface does not
 *       define whether the previous level was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should read the level back with
 *                    @c platform_hal_GetWebAccessLevel before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetWebAccessLevel
 *
 * @deprecated This function is scheduled for removal from this interface. New callers should not
 *             take a dependency on it.
 */
INT platform_hal_SetWebAccessLevel(INT userIndex, INT ifIndex, ULONG value);

/**
 * @brief Initialises the Platform and Management (PandM) database.
 *
 * Prepares the store that holds platform-specific configuration so that the functions depending on
 * it can be served. A vendor implementation may also start whatever internal threads it needs at
 * this point.
 *
 * @pre None established by this interface. This is one of the initialization functions the client
 *      is expected to call during the boot sequence, and the initialization functions may be called
 *      in any order relative to one another. This interface does not enumerate which of the other
 *      functions require it, so a caller that has no dependency on the PandM database need not call
 *      it. The effect of calling it a second time is not defined by this interface.
 * @post On success the PandM database is ready for use.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the initialization is successful.
 * @retval RETURN_ERR if any error occurs during initialization. The caller should treat the
 *                    platform configuration store as unavailable rather than proceeding.
 *
 * @execution Synchronous; the calling thread is blocked until initialization completes. This
 *            function is expected to block while the hardware is not yet ready, so a caller on a
 *            latency-sensitive path should not invoke it there.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_DocsisParamsDBInit
 */
INT platform_hal_PandMDBInit(void);

/**
 * @brief Initialises the DOCSIS parameters database.
 *
 * Prepares the store that holds DOCSIS cable-modem configuration and data so that the functions
 * depending on it can be served. A vendor implementation may also start whatever internal threads
 * it needs at this point.
 *
 * @pre None established by this interface. This is one of the initialization functions the client
 *      is expected to call during the boot sequence, and the initialization functions may be called
 *      in any order relative to one another. This interface does not enumerate which of the other
 *      functions require it. The effect of calling it a second time is not defined by this
 *      interface.
 * @post On success the DOCSIS parameters database is ready for use.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the initialization is successful.
 * @retval RETURN_ERR if any error occurs during initialization. The caller should treat the DOCSIS
 *                    parameter store as unavailable rather than proceeding.
 *
 * @execution Synchronous; the calling thread is blocked until initialization completes. This
 *            function is expected to block while the hardware is not yet ready, so a caller on a
 *            latency-sensitive path should not invoke it there.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_PandMDBInit
 */
INT platform_hal_DocsisParamsDBInit(void);

/**
 * @brief Reads the device model name.
 *
 * Returns the vendor-assigned model identifier for the board, which a caller can use to select
 * model-specific behaviour.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the model name as text. The value
 *                      is vendor specific. The buffer must be at least 256 bytes, the maximum length this
 *                      interface allows for a model name. This interface states neither how many bytes
 *                      the implementation writes nor whether it writes a terminator, so a caller must
 *                      bound every read to the 256 bytes it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator if the
 *                      implementation writes the whole buffer, so allocate one byte beyond the size this
 *                      interface requires and zero that byte before the call. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the model name. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the model name as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetModelName(CHAR *pValue);

/**
 * @brief Reads the regulatory region the router is provisioned for.
 *
 * Returns the region code that governs region-dependent behaviour such as regulatory limits.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the region name as text. Possible
 *                      values are `REGION_UK`, `REGION_IT`, `REGION_ROI` and `REGION_UNKNOWN`. The
 *                      buffer must be at least 256 bytes, the maximum length this interface allows for a
 *                      region name. This interface states neither how many bytes the implementation
 *                      writes nor whether it writes a terminator, so a caller must bound every read to
 *                      the 256 bytes it supplied and, where a terminated string is required, must do so
 *                      itself: zero-filling alone leaves no terminator if the implementation writes the
 *                      whole buffer, so allocate one byte beyond the size this interface requires and
 *                      zero that byte before the call. This interface does not specify whether the
 *                      implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the region code. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the region code as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetRouterRegion(CHAR *pValue);

/**
 * @brief Reads the device serial number.
 *
 * Returns the vendor-assigned serial number that uniquely identifies this unit.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the serial number as
 *                      text. The value is vendor specific. The buffer must be at least 256
 *                      bytes, the maximum length this interface allows for a serial number.
 *                      This interface states neither how many bytes the implementation writes
 *                      nor whether it writes a terminator, so a caller must bound every read
 *                      to the 256 bytes it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator
 *                      if the implementation writes the whole buffer, so allocate one byte
 *                      beyond the size this interface requires and zero that byte before the
 *                      call. This interface does not specify whether the implementation keeps
 *                      the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the serial number. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the serial number as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetSerialNumber(CHAR *pValue);

/**
 * @brief Reads the hardware revision of the device.
 *
 * Returns the vendor-assigned hardware version string, which a caller can use to distinguish
 * board revisions that behave differently.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the hardware version as
 *                      text. The value is vendor specific. The buffer must be at least 256
 *                      bytes, the maximum length this interface allows for a hardware version
 *                      string. This interface states neither how many bytes the implementation
 *                      writes nor whether it writes a terminator, so a caller must bound every
 *                      read to the 256 bytes it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator if
 *                      the implementation writes the whole buffer, so allocate one byte beyond
 *                      the size this interface requires and zero that byte before the call.
 *                      This interface does not specify whether the implementation keeps the
 *                      pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the hardware version. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the hardware version as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetHardwareVersion(CHAR *pValue);

/**
 * @brief Reads the software version currently flashed on the device.
 *
 * Returns the version of the firmware the device is running.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the software version as text.
 *                      The value is vendor specific. This interface states neither how many bytes
 *                      the implementation writes nor whether it writes a terminator, so a caller
 *                      must bound every read to the @c maxSize bytes it reported and, where a
 *                      terminated string is required, must do so itself: zero-filling alone leaves
 *                      no terminator if the implementation writes the whole buffer, so report a
 *                      @c maxSize one byte short of the allocation and zero the byte beyond it
 *                      before the call. This interface does not specify whether the implementation
 *                      keeps the pointer after the call returns.
 * @param[in] maxSize - Size in bytes of the buffer supplied in @c pValue, passed by value.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least @c maxSize bytes.
 *      This interface does not define the behaviour for a null pointer or for a
 *      @c maxSize that misreports the allocation.
 * @post On success the output holds the software version. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the software version as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning The implementation is expected to honour @c maxSize. A caller that reports a
 *          @c maxSize larger than the buffer it actually allocated for @c pValue has no
 *          defined protection against an overrun.
 */
INT platform_hal_GetSoftwareVersion(CHAR* pValue, ULONG maxSize);

/**
 * @brief Reads the bootloader version currently flashed on the device.
 *
 * Returns the version of the bootloader the device starts from.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the bootloader version as
 *                      text. The value is vendor specific. This interface states neither how many
 *                      bytes the implementation writes nor whether it writes a terminator, so a
 *                      caller must bound every read to the @c maxSize bytes it reported and, where a
 *                      terminated string is required, must do so itself: zero-filling alone leaves
 *                      no terminator if the implementation writes the whole buffer, so report a
 *                      @c maxSize one byte short of the allocation and zero the byte beyond it
 *                      before the call. This interface does not specify whether the implementation
 *                      keeps the pointer after the call returns.
 * @param[in] maxSize - Size in bytes of the buffer supplied in @c pValue, passed by value.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least @c maxSize bytes.
 *      This interface does not define the behaviour for a null pointer or for a
 *      @c maxSize that misreports the allocation.
 * @post On success the output holds the bootloader version. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the bootloader version as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning The implementation is expected to honour @c maxSize. A caller that reports a
 *          @c maxSize larger than the buffer it actually allocated for @c pValue has no
 *          defined protection against an overrun.
 */
INT platform_hal_GetBootloaderVersion(CHAR *pValue, ULONG maxSize);

/**
 * @brief Reads the name of the firmware image flashed on the device.
 *
 * Returns the image name of the running firmware, which a caller can compare against an expected
 * build.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the firmware image name as
 *                      text. The value is vendor specific. This interface states neither how many
 *                      bytes the implementation writes nor whether it writes a terminator, so a
 *                      caller must bound every read to the @c maxSize bytes it reported and, where a
 *                      terminated string is required, must do so itself: zero-filling alone leaves
 *                      no terminator if the implementation writes the whole buffer, so report a
 *                      @c maxSize one byte short of the allocation and zero the byte beyond it
 *                      before the call. This interface does not specify whether the implementation
 *                      keeps the pointer after the call returns.
 * @param[in] maxSize - Size in bytes of the buffer supplied in @c pValue, passed by value.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least @c maxSize bytes.
 *      This interface does not define the behaviour for a null pointer or for a
 *      @c maxSize that misreports the allocation.
 * @post On success the output holds the firmware image name. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the firmware image name as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning The implementation is expected to honour @c maxSize. A caller that reports a
 *          @c maxSize larger than the buffer it actually allocated for @c pValue has no
 *          defined protection against an overrun.
 */
INT platform_hal_GetFirmwareName(CHAR *pValue, ULONG maxSize);

/**
 * @brief Reads the base MAC address of the device.
 *
 * Returns the address from which the device derives the MAC addresses of its individual
 * interfaces.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving
 *                      the zero-terminated base MAC address, formatted as colon-separated
 *                      octets, for example AA:BB:CC:DD:EE:FF. The buffer must be at least
 *                      256 bytes, which this interface allows for the address text and any
 *                      terminator or additional information. This interface does not specify
 *                      whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the base MAC address. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the base MAC address as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetBaseMacAddress(CHAR *pValue);

/**
 * @brief Reads the total flash size of the device in megabytes.
 *
 * Returns the size of the whole flash device as decimal text rather than as a number, so a caller
 * that needs an integer must convert it.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the total flash size in
 *                      MB as decimal text, in the range 1 to 4096, which are the smallest and
 *                      largest sizes this interface allows. The buffer must be at least 256
 *                      bytes. This interface states neither how many bytes the implementation
 *                      writes nor whether it writes a terminator, so a caller must bound every
 *                      read to the 256 bytes it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator if
 *                      the implementation writes the whole buffer, so allocate one byte beyond
 *                      the size this interface requires and zero that byte before the call.
 *                      This interface does not specify whether the implementation keeps the
 *                      pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the total flash size. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the total flash size as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetHardware(CHAR *pValue);

/**
 * @brief Reads the used flash size of the device in megabytes.
 *
 * Returns the amount of flash currently occupied, as decimal text rather than as a number.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the used flash size in MB as
 *                      decimal text, in the range 1 to n inclusive, where n is the total flash size
 *                      of the device in MB. The buffer must be at least 1024 bytes and no more than
 *                      2048 bytes. This interface states neither how many bytes the implementation
 *                      writes nor whether it writes a terminator, so a caller must bound every read
 *                      to the buffer capacity it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator if the
 *                      implementation writes the whole buffer, so allocate one byte beyond the size
 *                      this interface requires and zero that byte before the call. This interface
 *                      does not specify whether the implementation keeps the pointer after the call
 *                      returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 1024 bytes, and no more than 2048 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the used flash size. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the used flash size as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetHardware_MemUsed(CHAR *pValue);

/**
 * @brief Reads the available flash size of the device in megabytes.
 *
 * Returns the amount of flash still free, as decimal text rather than as a number.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the
 *                      zero-terminated available flash size in MB, in the range 1 to n inclusive, where
 *                      n is the total flash size less the space in use. The returned string is zero
 *                      terminated. The buffer must be at least 1024 bytes. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 1024 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the available flash size. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the available flash size as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetHardware_MemFree(CHAR *pValue);

/**
 * @brief Reads the total DRAM size of the device in megabytes.
 *
 * Returns the total volatile memory fitted to the device, as a number.
 *
 * @param[out] pulSize - Caller-allocated, caller-owned @c ULONG receiving the
 *                      size in MB. The value lies in the range 1 to n inclusive, where n is the
 *                      largest DRAM size available on the device in MB. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pulSize must point to a caller-allocated buffer of at least one ULONG. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the total DRAM size. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the total DRAM size as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_GetTotalMemorySize(ULONG *pulSize);

/**
 * @brief Reads the DRAM currently in use, in megabytes.
 *
 * Returns how much volatile memory the system is currently occupying, as a number.
 *
 * @param[out] pulSize - Caller-allocated, caller-owned @c ULONG receiving the
 *                      size in MB. The value lies in the range 1 to n inclusive, where n is the
 *                      total DRAM size available on the device in MB. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pulSize must point to a caller-allocated buffer of at least one ULONG. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the used DRAM size. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the used DRAM size as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_GetUsedMemorySize(ULONG *pulSize);

/**
 * @brief Reads the DRAM currently free, in megabytes.
 *
 * Returns how much volatile memory remains available, as a number.
 *
 * @param[out] pulSize - Caller-allocated, caller-owned @c ULONG receiving the
 *                      size in MB. The value lies in the range 1 to n inclusive, where n is the
 *                      total DRAM size available on the device in MB. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pulSize must point to a caller-allocated buffer of at least one ULONG. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the available DRAM size. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the available DRAM size as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_GetFreeMemorySize(ULONG *pulSize);

/**
 * @brief Reads how many times the device has been factory reset.
 *
 * Reports the counter published as Device.DeviceInfo.FactoryResetCount.
 *
 * @param[out] pulSize - Caller-allocated, caller-owned @c ULONG receiving
 *                       the count, which is 0 or greater. This interface does not specify
 *                       whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pulSize must point to a caller-allocated buffer of at least one ULONG. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the factory reset count. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the count as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_ClearResetCount
 */
INT platform_hal_GetFactoryResetCount(ULONG *pulSize);

/**
 * @brief Clears the factory reset counter.
 *
 * Performs the operation published as Device.DeviceInfo.ClearResetCount.
 *
 * @param[in] bFlag - Passed by value. Pass @c TRUE to reset the counter; pass @c FALSE to leave
 *                    it unchanged, in which case the call has no effect on the counter.
 *
 * @pre None established by this interface. The valid values of @c bFlag are @c TRUE and @c FALSE.
 * @post On success the counter is zero when @c bFlag was @c TRUE, and unchanged when it was
 *       @c FALSE. On failure this interface does not define whether the counter changed.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should read the counter back with
 *                    @c platform_hal_GetFactoryResetCount before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetFactoryResetCount
 */
INT platform_hal_ClearResetCount(BOOLEAN bFlag);

/**
 * @brief Reads the device's time offset from UTC.
 *
 * Reports the value published as Device.DeviceInfo.TimeOffset, expressed in seconds relative to
 * UTC and returned as signed decimal text rather than as a number.
 *
 * @param[out] timeOffSet - Caller-allocated, caller-owned buffer receiving the offset in seconds as
 *                          signed decimal text. Examples of valid values are '-25200' (-7 hours,
 *                          Mountain Standard Time, UTC-7), '-18000' (-5 hours, Eastern Standard Time,
 *                          UTC-5, the default) and '+25200' (+7 hours, Indonesia Western Time, UTC+7).
 *                          The buffer must be at least 256 bytes. This interface states neither how
 *                          many bytes the implementation writes nor whether it writes a terminator, so
 *                          a caller must bound every read to the 256 bytes it supplied and, where a
 *                          terminated string is required, must do so itself: zero-filling alone leaves
 *                          no terminator if the implementation writes the whole buffer, so allocate one
 *                          byte beyond the size this interface requires and zero that byte before the
 *                          call. This interface does not specify whether the implementation keeps the
 *                          pointer after the call returns.
 *
 * @pre @c timeOffSet must point to a caller-allocated buffer of at least 256 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the offset as signed decimal text. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the time offset as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c timeOffSet but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_getTimeOffSet(CHAR *timeOffSet);

/**
 * @brief Sets the hardware watchdog timeout.
 *
 * Configures how long the hardware watchdog waits before it acts.
 *
 * @param[in] seconds - Timeout in seconds, passed by value. The accepted range is 0 to 3600
 *                      seconds inclusive, that is up to 60 minutes; the default is 60 minutes.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a value
 *      outside the accepted range, nor what a value of 0 selects.
 * @post On success the watchdog uses the requested timeout. On failure this interface does not
 *       define whether the previous timeout was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the watchdog timeout as
 *                    indeterminate and should not assume the requested value took effect.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_SetDeviceCodeImageTimeout(INT seconds);


/**
 * @brief Marks the flashed firmware image valid or invalid.
 *
 * Records whether the image currently flashed on the device should be treated as good, which
 * governs whether the platform keeps running it after a trial boot.
 *
 * @param[in] flag - Passed by value. Pass @c TRUE to mark the image valid and @c FALSE to mark it
 *                   not valid.
 *
 * @pre None established by this interface. The valid values of @c flag are @c TRUE and @c FALSE.
 * @post On success the image carries the requested validity marking. On failure this interface
 *       does not define whether the previous marking was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. Because this marking can decide whether the device
 *                    retains the running image, the caller should treat a failure as significant
 *                    and report it rather than retrying silently.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_SetDeviceCodeImageValid(BOOLEAN flag);

/**
 * @brief Reads the factory-programmed partner identifier.
 *
 * Returns the operator the unit was programmed for in the factory, which a caller can use to
 * select partner-specific behaviour.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the identifier as text.
 *                      Possible values are "unknown", "eUnprogrammed", "eComcast", "eCharter",
 *                      "eCox", "eRogers", "eVodafone", "eShaw" and "eVideotron". The buffer must be
 *                      at least 512 bytes. This interface states neither how many bytes the
 *                      implementation writes nor whether it writes a terminator, so a caller must
 *                      bound every read to the 512 bytes it supplied and, where a terminated string
 *                      is required, must do so itself: zero-filling alone leaves no terminator if
 *                      the implementation writes the whole buffer, so allocate one byte beyond the
 *                      size this interface requires and zero that byte before the call. This
 *                      interface does not specify whether the implementation keeps the pointer
 *                      after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 512 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds one of the listed identifiers. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the partner identifier as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_getFactoryPartnerId(CHAR *pValue);


/**
 * @brief Reads the factory-programmed cable-modem variant.
 *
 * Returns the provisioning variant the unit was programmed with in the factory.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the variant as text. Possible
 *                      values are "unknown", "pc20", "pc20genband", "pc15sip" and "pc15mgcp". The
 *                      buffer must be at least 512 bytes. This interface states neither how many bytes
 *                      the implementation writes nor whether it writes a terminator, so a caller must
 *                      bound every read to the 512 bytes it supplied and, where a terminated string is
 *                      required, must do so itself: zero-filling alone leaves no terminator if the
 *                      implementation writes the whole buffer, so allocate one byte beyond the size
 *                      this interface requires and zero that byte before the call. This interface does
 *                      not specify whether the implementation keeps the pointer after the call
 *                      returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 512 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds one of the listed variants. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the variant as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 * @see platform_hal_setFactoryCmVariant
 */
INT platform_hal_getFactoryCmVariant(CHAR *pValue);


/**
 * @brief Sets the factory-programmed cable-modem variant.
 *
 * Records the provisioning variant the unit should report.
 *
 * @param[in] pValue - Caller-allocated, caller-owned buffer holding the variant to apply, which the
 *                     caller must supply as a zero-terminated string; termination is the caller's obligation
 *                     on the value it passes in. Accepted values are "unknown", "pc20", "pc20genband",
 *                     "pc15sip" and "pc15mgcp". The string must not exceed 512 bytes. The buffer is read
 *                     during the call; this interface does not specify whether the implementation keeps the
 *                     pointer after the call returns.
 *
 * @pre @c pValue must point to a zero-terminated string of no more than 512 bytes. This interface
 *      does not define the behaviour for a null pointer, an over-length string, or a value outside
 *      the accepted set.
 * @post On success the device reports the requested variant. On failure this interface does not
 *       define whether the previous variant was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should read the variant back with
 *                    @c platform_hal_getFactoryCmVariant before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_getFactoryCmVariant
 */
INT platform_hal_setFactoryCmVariant(CHAR *pValue);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

#define LED_BUFFER_SIZE 3 /*!< @brief Number of LED entries a caller may address through the LED functions. */

/**
* @enum LED_COLOR
* @brief Defines the available LED color options for the device.
*/
typedef enum {
    LED_WHITE=0,                //!< Default LED color.
    LED_YELLOW=1,               //!< This is also classified as Amber from RDKB perspective.
    LED_GREEN=2,                //!< Green LED color.
    LED_RED=3,                  //!< Red LED color.
    LED_BLUE=4,                 //!< Blue LED color.
    LED_RED_AND_GREEN=5,        //!< Combination of Red and Green LED colors.
    LED_YELLOW_AND_GREEN=6,     //!< Combination of Yellow and Green LED colors.
    NOT_SUPPORTED=7             //!< Indicates the LED color is not supported.
} LED_COLOR;

/**
 * @struct _LEDMGMT_PARAMS
 * @brief Parameters for managing the LED behaviour on the device.
 *
 * The configuration structure passed to @c platform_hal_setLed and filled by
 * @c platform_hal_getLed. The @c led_name and @c led_param members exist only when
 * @c FEATURE_RDKB_LED_MANAGER is defined.
 */
typedef struct _LEDMGMT_PARAMS
{
#ifdef FEATURE_RDKB_LED_MANAGER
    /**
     * @brief Name of the LED the call acts on.
     *
     * This interface publishes no set of accepted names, so which names a platform recognises is
     * settled with the implementation rather than derived from this header. It establishes nothing
     * about the storage behind this pointer either: no maximum length, no allocation rule, no
     * guarantee that the text is zero-terminated, no statement of whether the caller or the
     * implementation owns it, and no statement of whether the member may be NULL.
     *
     * On input to @c platform_hal_setLed a caller therefore supplies its own storage and keeps it
     * valid for at least the duration of the call. On output from @c platform_hal_getLed a caller
     * cannot tell whether the implementation writes into storage the caller installed or replaces
     * the pointer with storage of its own, so it must not free the pointer it reads back, must not
     * write through it, and must not assume the text is bounded or terminated.
     */
    char * led_name;            /*!< Name of the LED to act on; this interface publishes no set of accepted names. */
    char * led_param;           /*!< @brief Additional parameters for the LED, in a vendor-defined form this interface does not describe. Length, allocation, termination, ownership and null handling are unspecified exactly as for @c led_name, so a caller must supply and retain its own storage on input and must treat anything it reads back as being of unknown length, unknown termination and unknown ownership. Because no capacity is stated for either member, a caller cannot size a buffer from this header and must obtain any bound it needs from the implementation it runs against. */
#endif
    LED_COLOR  LedColor;        //!< LED color choice from @ref LED_COLOR enum.
    INT  State;                 //!< LED state (0 for Solid, 1 for Blink).
    INT  Interval;              //!< Blink interval duration (in seconds) when LED state is set to Blink.
}
LEDMGMT_PARAMS, *PLEDMGMT_PARAMS;


/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

#ifdef FEATURE_RDKB_LED_MANAGER
/**
 * @brief Initialises the LED layer and reports the path of its configuration file.
 *
 * Prepares the LED subsystem for use and tells the caller which configuration file the vendor
 * implementation is driven by.
 *
 * @param[out] config_file_name - Caller-allocated, caller-owned buffer receiving the configuration file
 *                               name including its full path as text, for example
 *                               /usr/rdk/rdkledmanager/led_config_gb.json. The value is vendor specific.
 *                               The buffer must be at least 128 bytes. This interface states neither how
 *                               many bytes the implementation writes nor whether it writes a terminator,
 *                               so a caller must bound every read to the 128 bytes it supplied and,
 *                               where a terminated string is required, must do so itself: zero-filling
 *                               alone leaves no terminator if the implementation writes the whole
 *                               buffer, so allocate one byte beyond the size this interface requires and
 *                               zero that byte before the call. This interface does not specify whether
 *                               the implementation keeps the pointer after the call returns.
 *
 * @pre @c config_file_name must point to a caller-allocated buffer of at least 128 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the buffer holds the full path of the LED configuration file. On failure this
 *       interface does not define the buffer's contents.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the LED subsystem as
 *                    unavailable and should not call the other LED functions.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c config_file_name but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 * @note This function is declared only when @c FEATURE_RDKB_LED_MANAGER is defined.
 * @note Unlike every other function in this interface, this one is declared with the lowercase
 *       @c int return type rather than @c INT. The two are the same type here, because @c INT is
 *       a macro for @c int.
 * @see platform_hal_setLed
 * @see platform_hal_getLed
 */
int platform_hal_initLed(char *config_file_name);
#endif

/**
 * @brief Applies an LED configuration to the device.
 *
 * Sets the LED's colour, its steady or blinking state, and the blink interval, from a single
 * caller-supplied structure.
 *
 * @param[in] pValue - Caller-allocated, caller-owned @c LEDMGMT_PARAMS structure holding the configuration to apply. Every
 *                     member a caller sets is described on @c LEDMGMT_PARAMS, including the two @c char* members present when
 *                     @c FEATURE_RDKB_LED_MANAGER is defined, whose length, allocation, termination and ownership this interface
 *                     leaves unspecified. The structure is read during the call; this interface does not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a fully populated @c LEDMGMT_PARAMS structure. This interface does
 *      not define the behaviour for a null pointer or for a member outside its documented range.
 * @post On success the LED shows the requested colour and state. On failure this interface does
 *       not define whether the LED changed at all or changed only in part.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the LED behaviour was successfully set.
 * @retval RETURN_ERR if any error occurred during the operation. The caller should read the state
 *                    back with @c platform_hal_getLed before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_getLed
 * @see LEDMGMT_PARAMS
 */
INT platform_hal_setLed(PLEDMGMT_PARAMS pValue);

/**
 * @brief Reads the LED configuration in force at the time of the call.
 *
 * Reports the LED's current colour, state and blink interval.
 *
 * @param[out] pValue - Caller-allocated, caller-owned @c LEDMGMT_PARAMS structure that the implementation fills with the
 *                      current configuration. The caller allocates the structure itself, so this argument is an out parameter
 *                      of pointer type rather than a pointer to a pointer. Where @c FEATURE_RDKB_LED_MANAGER is defined the
 *                      structure carries two @c char* members whose storage, length, termination and ownership this interface
 *                      leaves unspecified; see @c LEDMGMT_PARAMS. This interface does not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated @c LEDMGMT_PARAMS structure. This interface does
 *      not define the behaviour for a null pointer.
 * @post On success the output holds the current LED configuration. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the LED configuration as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_setLed
 * @see LEDMGMT_PARAMS
 */
INT platform_hal_getLed(PLEDMGMT_PARAMS pValue);

/**
 * @brief Reads the PWM-derived speed of a fan.
 *
 * Reports the fan's current speed as derived from its pulse-width-modulation setting. The result
 * is the actual speed reading rather than the raw PWM duty value.
 *
 * @param[in] fanIndex - Index of the fan to query, passed by value. The valid range is 0 to 1
 *                       inclusive, so only two fans are addressable.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a
 *      @c fanIndex of 2 or above.
 * @post None. This function reports a reading and changes no platform state.
 *
 * @return The fan's speed, in the range 0 to n inclusive, where n represents either the maximum
 *         rotation rate or the value standing for 100% of maximum speed.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Both the unit of the returned value and the upper bound n depend on the OEM's fan speed
 *       specification, so a caller must not treat the figure as a fixed-scale percentage.
 * @note This function returns a reading rather than a status code, and this interface defines no
 *       distinguished value to signal failure. A caller therefore cannot separate a genuine
 *       reading of 0 from a failed query.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_getRPM
 * @see platform_hal_getFanStatus
 */
UINT platform_hal_getFanSpeed(UINT fanIndex);

/**
 * @brief Reads the rotation rate of a fan.
 *
 * Reports the fan's current rate in revolutions per minute.
 *
 * @param[in] fanIndex - Index of the fan to query, passed by value. The valid range is 0 to 1
 *                       inclusive, so @c fanIndex must be less than 2.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a
 *      @c fanIndex of 2 or above.
 * @post None. This function reports a reading and changes no platform state.
 *
 * @return The fan's rotation rate in revolutions per minute, in the range 0 to n inclusive.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note The upper bound n is OEM dependent, so a caller must not assume a fixed maximum.
 * @note This function returns a reading rather than a status code, and this interface defines no
 *       distinguished value to signal failure. A caller therefore cannot separate a genuine
 *       reading of 0, which a stopped fan would also give, from a failed query.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_getFanSpeed
 * @see platform_hal_getRotorLock
 */
UINT platform_hal_getRPM(UINT fanIndex);

/**
 * @brief Reads the rotor lock status of a fan.
 *
 * Reports whether the fan's rotor is stalled, which is the condition a caller uses to detect a
 * seized or obstructed fan.
 *
 * @param[in] fanIndex - Index of the fan to query, passed by value. The valid range is 0 to 1
 *                       inclusive, so @c fanIndex must be less than 2.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a
 *      @c fanIndex of 2 or above.
 * @post None. This function reports a reading and changes no platform state.
 *
 * @return The rotor lock indication for the selected fan, as one of the three values this interface
 *         defines for it: 1 when the rotor is locked, 0 when the rotor is not locked, and -1 when the
 *         indication is not applicable, for instance where the platform or the selected fan provides
 *         none. A caller must read -1 as "unknown" rather than as "not locked". This interface defines
 *         no fourth value and no distinguished value through which this function reports a failed
 *         query, so a caller cannot separate an unavailable indication from a failure.
 * @retval 1 The rotor of the selected fan is locked.
 * @retval 0 The rotor of the selected fan is not locked.
 * @retval -1 The indication is not applicable - for instance where the platform or the selected
 *            fan provides none. A caller reads this as "unknown" rather than as "not locked".
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note This function reports a reading of its own rather than a code from the @c RETURN_OK and @c RETURN_ERR
 *       family, so its result is documented as a value domain and not as a status list. The value -1 carries
 *       the meaning documented above; this interface defines no separate error code for this function, so -1
 *       must not be read as a generic failure even though it shares the numeric value of @c RETURN_ERR.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_getRPM
 */
INT platform_hal_getRotorLock(UINT fanIndex);

/**
 * @brief Reads whether a fan is enabled.
 *
 * Reports the fan's enabled state, which a caller checks before interpreting a speed or rotation
 * reading.
 *
 * @param[in] fanIndex - Index of the fan to query, passed by value. The valid range is 0 to 1
 *                       inclusive, so @c fanIndex must be less than 2.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a
 *      @c fanIndex of 2 or above.
 * @post None. This function reports a reading and changes no platform state.
 *
 * @return @c TRUE if the fan is enabled, @c FALSE if it is disabled.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note This function returns a state rather than a status code, and this interface defines no
 *       distinguished value to signal failure. A caller therefore cannot separate a disabled fan
 *       from a failed query.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_getFanSpeed
 */
BOOLEAN platform_hal_getFanStatus(UINT fanIndex);

/**
 * @brief Forces a fan to maximum speed, or releases it back to normal control.
 *
 * Overrides the platform's own fan control so the fan runs flat out, and reverses that override
 * when it is no longer wanted.
 *
 * @param[in] bOverrideFlag - Passed by value. Pass @c TRUE to force the fan to maximum speed and
 *                            @c FALSE to restore its normal, platform-controlled behaviour.
 * @param[in] fanIndex - Index of the fan the override applies to, passed by value. The valid range
 *                       is 0 to 1 inclusive, so only two fans are addressable.
 *
 * @pre None established by this interface. This interface does not define the behaviour for a
 *      @c fanIndex of 2 or above.
 * @post On success the fan is in the requested mode. On failure this interface does not define
 *       whether the previous mode was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the fan was successfully placed into, or released from, high-speed mode.
 * @retval RETURN_ERR if an error occurs. Because leaving a fan under an unintended override has
 *                    thermal consequences, the caller should report the failure rather than
 *                    assuming the previous mode still holds.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_setFanMaxOverride(BOOLEAN bOverrideFlag, UINT fanIndex);

#ifdef FEATURE_RDKB_THERMAL_MANAGER

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @enum FAN_SPEED
 * @brief Enumerates the various speed settings for a fan.
 */
typedef enum {
   FAN_SPEED_OFF=0,      /**< Represents the fan being turned off. */
   FAN_SPEED_SLOW=1,     /**< Represents a slow fan speed. */
   FAN_SPEED_MEDIUM=2,   /**< Represents a medium fan speed. */
   FAN_SPEED_FAST=3,     /**< Represents a fast fan speed. */
   FAN_SPEED_MAX=4       /**< Represents the maximum speed setting for the fan. */
} FAN_SPEED;

/**
 * @struct _FAN_PLATFORM_CONFIG
 * @brief Contains configuration settings related to fan operation in a platform.
 *
 * This structure holds various thresholds and timings that dictate how fans operate and
 * how their speed changes in response to different conditions.
 */
typedef struct _FAN_PLATFORM_CONFIG
{
   UINT FanCount;          /**< Total number of fans in the system. Can be either 1 or 2. */
   UINT SlowSpeedThresh;   /**< Temperature threshold (in °C) for classifying a fan speed as "slow". Typically set to 30°C. */
   UINT MediumSpeedThresh; /**< Temperature threshold (in °C) for classifying a fan speed as "medium". Typically set to 35°C. */
   UINT FastSpeedThresh;   /**< Temperature threshold (in °C) for classifying a fan speed as "fast". Typically set to 40°C. */
   UINT FanMinRunTime;     /**< The minimum runtime (in seconds) a fan has to run before it can be turned off. Usually set to 30 seconds. */
   UINT MonitoringDelay;   /**< Delay (in seconds) between monitoring checks for fan speed and conditions. */
   UINT PowerMonitoring;   /**< Power monitoring configuration for the fans. */
   UINT LogInterval;       /**< Interval (in seconds) at which fan-related data should be logged. */
} THERMAL_PLATFORM_CONFIG;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Initialises the thermal layer and reports the platform's fan configuration.
 *
 * Prepares the thermal subsystem for use and hands back the thresholds and timings that govern how
 * the platform's fans are driven. A vendor implementation may also start whatever internal threads
 * it needs at this point.
 *
 * @param[out] pThermalPlatformConfig - Caller-allocated,
 *                                      caller-owned @c THERMAL_PLATFORM_CONFIG structure that the
 *                                      implementation fills with the platform's thermal configuration.
 *                                      Each member is described on @c THERMAL_PLATFORM_CONFIG. This
 *                                      interface does not specify whether the implementation keeps the
 *                                      pointer after the call returns.
 *
 * @pre @c pThermalPlatformConfig must point to a caller-allocated @c THERMAL_PLATFORM_CONFIG
 *      structure. This is one of the initialization functions the client is expected to call
 *      during the boot sequence, and the initialization functions may be called in any order
 *      relative to one another. On a build without @c FEATURE_RDKB_THERMAL_MANAGER this function
 *      does not exist, so only the other two initialization functions are available.
 * @post On success the output holds the platform thermal configuration. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK on successful initialisation.
 * @retval RETURN_ERR if any error is detected. The caller should treat the thermal subsystem as
 *                    unavailable and should not call the other thermal functions.
 *
 * @execution Synchronous; the calling thread is blocked until initialization completes. This
 *            function is expected to block while the hardware is not yet ready.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @see platform_hal_LoadThermalConfig
 * @see THERMAL_PLATFORM_CONFIG
 */
INT platform_hal_initThermal(THERMAL_PLATFORM_CONFIG *pThermalPlatformConfig);

/**
 * @brief Loads the default thermal thresholds into a configuration structure.
 *
 * Fills a caller-supplied structure with the platform's default fan thresholds and timings, which
 * lets a caller recover the defaults without restarting the thermal subsystem.
 *
 * @param[out] pThermalPlatformConfig - Caller-allocated,
 *                                      caller-owned @c THERMAL_PLATFORM_CONFIG structure that the
 *                                      implementation fills with the default thermal configuration.
 *                                      Each member is described on @c THERMAL_PLATFORM_CONFIG. This
 *                                      interface does not specify whether the implementation keeps
 *                                      the pointer after the call returns.
 *
 * @pre @c pThermalPlatformConfig must point to a caller-allocated @c THERMAL_PLATFORM_CONFIG
 *      structure. This interface does not define whether this function may be called before
 *      @c platform_hal_initThermal, so a caller must not depend on either ordering.
 * @post On success the output holds the default thermal configuration. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK on successful loading of the thermal configuration.
 * @retval RETURN_ERR if any error is detected. The caller should treat the defaults as unknown
 *                    rather than substituting values of its own.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @see platform_hal_initThermal
 * @see THERMAL_PLATFORM_CONFIG
 */
INT platform_hal_LoadThermalConfig(THERMAL_PLATFORM_CONFIG *pThermalPlatformConfig);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @enum FAN_ERR
 * @brief Enumerates the possible error states for fan operations.
 */
typedef enum {
   FAN_ERR_NONE = 0,          /**< No error, the operation was successful. */
   FAN_ERR_HW = 1,            /**< Hardware error related to the fan. */
   FAN_ERR_MAX_OVERRIDE_SET = 2 /**< Error indicating that maximum override is already set. */
} FAN_ERR;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Sets a fan to one of the platform's discrete speed settings.
 *
 * Drives the selected fan at one of the enumerated speeds and, where the request cannot be
 * honoured, reports the reason through a separate output argument.
 *
 * @param[in] fanIndex - Index of the fan to set, passed by value. The valid range is 0 to 1
 *                       inclusive, so @c fanIndex must be less than 2.
 * @param[in] fanSpeed - Desired speed, passed by value. The value must be one of the
 *                       @c FAN_SPEED enumerators.
 * @param[out] pErrReason - Caller-allocated, caller-owned @c FAN_ERR that the
 *                          implementation fills with the reason the request could not be honoured. The
 *                          value is one of the @c FAN_ERR enumerators. This interface does not specify
 *                          whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pErrReason must point to a caller-allocated @c FAN_ERR. This interface does not define
 *      the behaviour for a null pointer, a @c fanIndex of 2 or above, or a @c fanSpeed outside the
 *      @c FAN_SPEED enumeration.
 * @post On success the fan runs at the requested speed. This interface does not define the value
 *       of @c pErrReason on success, nor whether the fan speed changed on failure, so a caller
 *       should read @c pErrReason only after a failing return.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the fan speed was successfully set.
 * @retval RETURN_ERR if any error is detected. The caller should inspect @c pErrReason to
 *                    distinguish a hardware fault from a speed override that is already in force,
 *                    and should not retry blindly in the latter case.
 * @retval RETURN_UNSUPPORTED if the platform does not implement this function. Unlike
 *                    @c RETURN_ERR this outcome is permanent for the platform, so the caller
 *                    should stop polling rather than retry. This code was added to the
 *                    interface in release 1.3.3 and is returned only by the functions whose
 *                    documentation lists it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @see FAN_SPEED
 * @see FAN_ERR
 * @see platform_hal_setFanMaxOverride
 */
INT platform_hal_setFanSpeed(UINT fanIndex, FAN_SPEED fanSpeed, FAN_ERR *pErrReason);

/**
 * @brief Reads the current device temperature.
 *
 * Reports the platform's temperature, which a caller uses to decide whether thermal mitigation is
 * needed.
 *
 * @param[out] pTemp - Caller-allocated, caller-owned @c INT receiving the
 *                     temperature in degrees Celsius. The valid range is 0 to 100 as defined
 *                     by the platform's requirements. This interface does not specify whether
 *                     the implementation keeps the pointer after the call returns.
 *
 * @pre @c pTemp must point to a caller-allocated buffer of at least one INT. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the temperature in degrees Celsius. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK on successfully retrieving the temperature reading.
 * @retval RETURN_ERR if any error is detected. The caller should treat the temperature as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 * @retval RETURN_UNSUPPORTED if the platform does not implement this function. Unlike
 *                    @c RETURN_ERR this outcome is permanent for the platform, so the caller
 *                    should stop polling rather than retry. This code was added to the
 *                    interface in release 1.3.3 and is returned only by the functions whose
 *                    documentation lists it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 */
INT platform_hal_getFanTemperature(INT *pTemp);

/* platform_hal_getInputCurrent() function */
/**
 * @brief Reads the platform's input current.
 *
 * Reports the current drawn at the platform's power input.
 *
 * @param[out] pValue - Caller-allocated, caller-owned @c INT receiving the input
 *                      current in milli-amperes. The valid range is 0 to 20000 mA. This interface does
 *                      not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least one INT. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the input current in milli-amperes. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the input current as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 * @retval RETURN_UNSUPPORTED if the platform does not implement this function. Unlike
 *                    @c RETURN_ERR this outcome is permanent for the platform, so the caller
 *                    should stop polling rather than retry. This code was added to the
 *                    interface in release 1.3.3 and is returned only by the functions whose
 *                    documentation lists it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @see platform_hal_getInputPower
 */
INT platform_hal_getInputCurrent(INT *pValue);

/**
 * @brief Reads the platform's input power.
 *
 * Reports the power consumed at the platform's power input.
 *
 * @param[out] pValue - Caller-allocated, caller-owned @c INT receiving the input
 *                      power in milli-watts. The valid range is 0 to 20000 mW. This interface does not
 *                      specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least one INT. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the input power in milli-watts. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the input power is successfully retrieved.
 * @retval RETURN_ERR if any error is detected. The caller should treat the input power as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 * @retval RETURN_UNSUPPORTED if the platform does not implement this function. Unlike
 *                    @c RETURN_ERR this outcome is permanent for the platform, so the caller
 *                    should stop polling rather than retry. This code was added to the
 *                    interface in release 1.3.3 and is returned only by the functions whose
 *                    documentation lists it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @see platform_hal_getInputCurrent
 */
INT platform_hal_getInputPower(INT *pValue);

/**
 * @brief Reads the temperature of one radio module.
 *
 * Reports the temperature of the radio selected by index, which a caller uses to apply
 * per-radio thermal mitigation.
 *
 * @param[in] radioIndex - Index of the radio to query, passed by value. The valid values are 0 for
 *                         the 2.4GHz radio, 1 for the 5GHz low-band radio and 2 for the 5GHz
 *                         high-band radio.
 * @param[out] pValue - Caller-allocated, caller-owned @c INT receiving the
 *                      temperature in degrees Celsius. The valid range is 0 to 100. This interface does
 *                      not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least one INT. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected. This interface does not define the
 *      behaviour for a @c radioIndex of 3 or above, nor for an index that names a radio the
 *      platform does not fit.
 * @post On success the output holds the radio temperature in degrees Celsius. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the radio temperature is successfully retrieved.
 * @retval RETURN_ERR if any error is detected. The caller should treat the radio temperature as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 * @retval RETURN_UNSUPPORTED if the platform does not implement this function. Unlike
 *                    @c RETURN_ERR this outcome is permanent for the platform, so the caller
 *                    should stop polling rather than retry. This code was added to the
 *                    interface in release 1.3.3 and is returned only by the functions whose
 *                    documentation lists it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @note The radio index range here differs from the one accepted by
 *       @c platform_hal_getEcoModeStatus, which recognises only 0 and 1.
 */
INT platform_hal_getRadioTemperature(INT radioIndex, INT *pValue);


/**
 * @brief Reads the ECO mode status of one radio module.
 *
 * Reports whether power-saving ECO mode is currently active on the radio selected by index.
 *
 * @param[in] radioIndex - Index of the radio to query, passed by value. The valid values are 0 for
 *                         the 2.4GHz radio and 1 for the 5GHz radio.
 * @param[out] pValue - Caller-allocated, caller-owned @c INT receiving the ECO mode
 *                      status, where 0 means ECO mode is disabled and 1 means it is enabled. This interface
 *                      does not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least one INT. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected. This interface does not define the
 *      behaviour for a @c radioIndex of 2 or above.
 * @post On success the output holds 0 or 1. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the ECO mode status is successfully retrieved.
 * @retval RETURN_ERR if any error is detected. The caller should treat the ECO mode status as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 * @retval RETURN_UNSUPPORTED if the platform does not implement this function. Unlike
 *                    @c RETURN_ERR this outcome is permanent for the platform, so the caller
 *                    should stop polling rather than retry. This code was added to the
 *                    interface in release 1.3.3 and is returned only by the functions whose
 *                    documentation lists it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note This function is declared only when @c FEATURE_RDKB_THERMAL_MANAGER is defined.
 * @note This function accepts a narrower radio index range than
 *       @c platform_hal_getRadioTemperature, which also recognises index 2.
 */
INT platform_hal_getEcoModeStatus(INT radioIndex, INT *pValue);

#endif

/**
 * @brief Allows or suppresses SNMP-initiated reboots during onboarding.
 *
 * Controls whether the platform acts on an SNMP reboot request while onboarding is in progress.
 *
 * @param[in] pValue - Caller-allocated, caller-owned buffer holding the
 *                     zero-terminated setting to apply. Accepted values are "disable" and "enable",
 *                     matched without regard to case. The string must not exceed 8 bytes including
 *                     its terminator. The buffer is read during the call; this interface does not
 *                     specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pValue must point to a zero-terminated string of no more than 8 bytes. This interface
 *      does not define the behaviour for a null pointer, an over-length string, or a value outside
 *      the accepted set.
 * @post On success the requested setting is in force. On failure this interface does not define
 *       whether the previous setting was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. Because a wrong setting here can let an SNMP
 *                    request reboot the device mid-onboarding, the caller should report the
 *                    failure rather than assume the request took effect.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 */
INT platform_hal_SetSNMPOnboardRebootEnable(CHAR *pValue);


/**
 * @brief Reads whether MACsec is enabled on a port.
 *
 * Reports the configured MACsec setting for one port, which is the administrative intent rather
 * than the state the driver has actually reached.
 *
 * @param[in] ethPort - Zero-based port index, passed by value. The index spans all interface
 *                      kinds together, including MoCA, Ethernet and Wi-Fi. The valid range is 0
 *                      to MaxEthPort-1, where MaxEthPort is platform specific and is not
 *                      published by this interface.
 * @param[out] pFlag - Caller-allocated, caller-owned @c BOOLEAN receiving the result,
 *                     where @c TRUE means MACsec is enabled and @c FALSE means it is not. This interface
 *                     does not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pFlag must point to a caller-allocated buffer of at least one BOOLEAN. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected. This interface does not define the behaviour for an
 *      @c ethPort at or above the platform's MaxEthPort, and provides no way to discover that
 *      limit.
 * @post On success the output holds @c TRUE or @c FALSE. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the MACsec state as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.

 * @note This reports configuration. Use @c platform_hal_GetMACsecOperationalStatus to learn
 *       whether MACsec is actually configured at the interface or driver level.
 * @see platform_hal_SetMACsecEnable
 * @see platform_hal_GetMACsecOperationalStatus
 */
INT platform_hal_GetMACsecEnable(INT ethPort, BOOLEAN *pFlag);

/**
 * @brief Enables or disables MACsec on a port.
 *
 * Records the administrative MACsec setting for one port. The driver may take time to converge, so
 * the operational state can lag this setting.
 *
 * @param[in] ethPort - Zero-based port index, passed by value. The index spans all interface kinds
 *                      together, including MoCA, Ethernet and Wi-Fi. The valid range is 0 to
 *                      MaxEthPort-1, where MaxEthPort is platform specific and is not published by
 *                      this interface.
 * @param[in] Flag - Passed by value. Pass @c TRUE to enable MACsec and @c FALSE to disable it.
 *
 * @pre None established by this interface. This interface does not define the behaviour for an
 *      @c ethPort at or above the platform's MaxEthPort.
 * @post On success the port carries the requested administrative setting. This interface does not
 *       define how long the operational state takes to follow, so a caller that needs the
 *       operational state should poll @c platform_hal_GetMACsecOperationalStatus.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should read the setting back with
 *                    @c platform_hal_GetMACsecEnable before relying on it.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetMACsecEnable
 * @see platform_hal_GetMACsecOperationalStatus
 */
INT platform_hal_SetMACsecEnable(INT ethPort, BOOLEAN Flag);

/**
 * @brief Reads whether MACsec is actually configured at the interface or driver level.
 *
 * Reports the operational MACsec state for one port, which may differ from the administrative
 * setting while the driver is still converging.
 *
 * @param[in] ethPort - Zero-based port index, passed by value. The index spans all interface
 *                      kinds together, including MoCA, Ethernet and Wi-Fi. The valid range is 0
 *                      to MaxEthPort-1, where MaxEthPort is platform specific and is not
 *                      published by this interface.
 * @param[out] pFlag - Caller-allocated, caller-owned @c BOOLEAN receiving the result,
 *                     where @c TRUE means MACsec is operational and @c FALSE means it is not. This interface
 *                     does not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pFlag must point to a caller-allocated buffer of at least one BOOLEAN. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected. This interface does not define the behaviour for an
 *      @c ethPort at or above the platform's MaxEthPort, and provides no way to discover that
 *      limit.
 * @post On success the output holds @c TRUE or @c FALSE. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the MACsec state as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.

 * @note This reports the operational state. Use @c platform_hal_GetMACsecEnable to read the
 *       administrative setting.
 * @see platform_hal_GetMACsecEnable
 */
INT platform_hal_GetMACsecOperationalStatus(INT ethPort, BOOLEAN *pFlag);

/**
 * @brief Starts MACsec on a port and waits for the outcome.
 *
 * Brings MACsec up on one port. This call is blocking: it does not return until the operation has
 * finished or its timeout has expired.
 *
 * @param[in] ethPort - Zero-based port index, passed by value. The index spans all interface kinds
 *                      together, including MoCA, Ethernet and Wi-Fi. The valid range is 0 to
 *                      MaxEthPort-1, where MaxEthPort is platform specific and is not published by
 *                      this interface.
 * @param[in] timeoutSec - Start timeout in seconds, passed by value, where 0 means retry forever.
 *                         This argument is currently unused and the only value passed is 0.
 *
 * @pre None established by this interface. This interface does not define the behaviour for an
 *      @c ethPort at or above the platform's MaxEthPort.
 * @post On success MACsec has been started on the port. On failure this interface does not define
 *       whether MACsec was left partially started.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should confirm the port's state with
 *                    @c platform_hal_GetMACsecOperationalStatus before retrying.
 *
 * @execution Synchronous and blocking by design; the calling thread is blocked until MACsec has
 *            started or the operation gives up. Because @c timeoutSec is unused and 0 means retry
 *            forever, this interface publishes no upper bound on how long the call may take, so a
 *            caller must not invoke it on a latency-sensitive path.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_StopMACsec
 * @see platform_hal_GetMACsecOperationalStatus
 */
INT platform_hal_StartMACsec(INT ethPort, INT timeoutSec);

/**
 * @brief Stops MACsec on a port and waits for the outcome.
 *
 * Takes MACsec down on one port. This call is blocking: it does not return until the operation has
 * finished.
 *
 * @param[in] ethPort - Zero-based port index, passed by value. The index spans all interface kinds
 *                      together, including MoCA, Ethernet and Wi-Fi. The valid range is 0 to
 *                      MaxEthPort-1, where MaxEthPort is platform specific and is not published by
 *                      this interface.
 *
 * @pre None established by this interface. This interface does not define the behaviour for an
 *      @c ethPort at or above the platform's MaxEthPort.
 * @post On success MACsec has been stopped on the port. On failure this interface does not define
 *       whether MACsec was left partially stopped.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should confirm the port's state with
 *                    @c platform_hal_GetMACsecOperationalStatus before retrying.
 *
 * @execution Synchronous and blocking by design; the calling thread is blocked until MACsec has
 *            stopped. This interface publishes no upper bound on how long the call may take.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_StartMACsec
 */
INT platform_hal_StopMACsec(INT ethPort);

/**
 * @brief Reads the DRAM and eMMC storage paths of a given processor.
 *
 * Reports the filesystem paths a processor uses for volatile and persistent storage, so a caller
 * can place files without hard-coding platform layout.
 *
 * @param[in] index - Processor to query, passed by value. The value must be one of the
 *                    @c RDK_CPUS enumerators; @c HOST_CPU is the primary RDK processor.
 * @param[out] ppinfo - Address of a caller-held @c PPLAT_PROC_MEM_INFO pointer. The
 *                      implementation allocates a @c PLAT_PROC_MEM_INFO array, fills it, and
 *                      writes its address here. The caller supplies only the pointer variable,
 *                      not the array.
 *
 * @pre @c ppinfo must be the address of a valid pointer variable. This interface does not define
 *      the behaviour for a null @c ppinfo or for an @c index outside @c RDK_CPUS.
 * @post On success @c *ppinfo refers to an implementation-allocated array whose members are
 *       described on @c PLAT_PROC_MEM_INFO. On failure this interface does not define whether
 *       @c *ppinfo was written or whether anything was allocated, so a caller must not free it.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the memory paths as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes, the allocation below included. This interface publishes no numeric time bound for it.
 * @note This function is the one documented exception to the memory model described in
 *       @ref platform_hal "the Platform HAL runtime contract". Here the HAL allocates the array
 *       of @c PLAT_PROC_MEM_INFO and returns it through @c ppinfo, rather than filling a buffer
 *       the caller allocated.
 * @warning This interface does not state which side releases the array the HAL allocated, and it
 *          declares no matching free function. A caller can therefore neither free it safely nor
 *          assume the HAL will, so the ownership of that allocation is unspecified and must be
 *          settled with the implementer before the result is relied on in a long-running process.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see RDK_CPUS
 * @see PLAT_PROC_MEM_INFO
 */
INT platform_hal_GetMemoryPaths(RDK_CPUS index, PPLAT_PROC_MEM_INFO *ppinfo);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @struct dhcp_opt_list
 * @brief Singly linked list node carrying one DHCPv4 or DHCPv6 option and its value.
 *
 * The structure represents a linked list, where each node contains
 * an option and its corresponding value.
 *
 * A caller meets this type only as the head of a list produced by
 * @c platform_hal_GetDhcpv4_Options or @c platform_hal_GetDhcpv6_Options, and the numbering space
 * of @c dhcp_opt follows whichever of the two produced the list: a node does not record the
 * protocol it belongs to. What this interface does not establish about such a list is as
 * important to a caller as what it does. It does not state which side allocated the nodes or the
 * values they point at, it declares no function that releases either, and neither the end of the
 * list nor the extent of a value is fixed by the members below - so this interface specifies no
 * safe way to traverse a returned list and no way to release it. A caller settles the termination
 * rule, the value representation and the release rule with the implementation it runs against
 * before it dereferences, copies or frees anything reached through the head pointer. The three
 * members are documented on those terms.
 */
typedef struct dhcp_opt_list {
    /**
     * @brief DHCP option code carried by this node.
     *
     * The header names a constant for each code this interface calls out: @c DHCPV6_OPT_3,
     * @c DHCPV6_OPT_5, @c DHCPV6_OPT_15, @c DHCPV6_OPT_16, @c DHCPV6_OPT_17, @c DHCPV6_OPT_20,
     * @c DHCPV6_OPT_22, @c DHCPV6_OPT_23, @c DHCPV6_OPT_24, @c DHCPV6_OPT_25, @c DHCPV6_OPT_31,
     * @c DHCPV6_OPT_64, @c DHCPV6_OPT_82, @c DHCPV6_OPT_83 and @c DHCPV6_OPT_95 on the DHCPv6
     * side, and @c DHCPV4_OPT_2, @c DHCPV4_OPT_42, @c DHCPV4_OPT_43, @c DHCPV4_OPT_58,
     * @c DHCPV4_OPT_59, @c DHCPV4_OPT_60, @c DHCPV4_OPT_61, @c DHCPV4_OPT_100, @c DHCPV4_OPT_122,
     * @c DHCPV4_OPT_125, @c DHCPV4_OPT_242, @c DHCPV4_OPT_243 and @c DHCPV4_OPT_END on the DHCPv4
     * side. Each constant equals the registered code of the option its own comment names; the
     * DHCPv4 codes are those defined by RFC 2132 and the IANA registry that continues it, so the
     * meaning of a code is fixed there and not by this interface.
     *
     * @c DHCPV4_OPT_END is the RFC 2132 End option, code 255, which the macro's comment describes
     * as the value used to check whether an option is valid; this interface does not state that a
     * node carrying it ends a list.
     *
     * The member is not restricted to the constants above: each protocol's code space is larger
     * than the set named here, so a caller must be prepared for a code it has no constant for and
     * must not read an unrecognised value as corruption.
     */
    int dhcp_opt; /*!< DHCP option code for this node, held as a plain @c int and not restricted to the named option constants. */
    /**
     * @brief Pointer to the value associated with the DHCP option code.
     *
     * This interface states neither how long that value is nor how a caller determines its length:
     * the member is a bare @c char pointer, no length travels beside it, the option code does not
     * fix a length for every option, and nothing here establishes that the bytes are
     * NUL-terminated text rather than a counted binary value - several of the options named for
     * @c dhcp_opt carry binary payloads. The readable extent of the value is therefore
     * unspecified, as is whether the pointer may be NULL for an option that carries no value.
     *
     * A caller must not dereference it, and must not copy or print it with an unbounded string
     * operation, until it has established the representation and the length rule with the
     * implementation it runs against. Which side allocated the value and which side releases it
     * are equally unstated; see the description of @c dhcp_opt_list.
     */
    char * dhcp_opt_val; /*!< Value bytes for @c dhcp_opt; no length travels beside the pointer, so the readable extent is unspecified. */
    struct dhcp_opt_list * next; /*!< Pointer to the next DHCP option in the list. This interface does not state how the list ends: it does not say that the last node holds NULL here, and it publishes no node count, no length accessor and no other terminator, so a caller cannot derive a stopping condition for a traversal from this header alone. Whether the nodes were allocated by the implementation or by the caller is not stated either, and no function declared here releases them; see the description above. */
} dhcp_opt_list;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Reads the DHCPv4 options the platform requests and sends.
 *
 * Builds two lists for the DHCPv4 client: the options the platform asks the server for, and the
 * options the platform includes in its own messages.
 *
 * @param[out] req_opt_list - Address of a caller-held @c dhcp_opt_list pointer. The
 *                            implementation builds the list of requested DHCPv4 options and writes
 *                            its head here. The codes it carries are DHCPv4 codes, for which the
 *                            header defines the @c DHCPV4_OPT_ constants; @c dhcp_opt_list states
 *                            what the member does and does not restrict.
 * @param[out] send_opt_list - Address of a caller-held @c dhcp_opt_list pointer. The
 *                             implementation builds the list of DHCPv4 options to be sent and
 *                             writes its head here, on the same terms as @c req_opt_list.
 *
 * @pre Both arguments must be addresses of valid pointer variables. This interface does not define
 *      the behaviour for a null argument.
 * @post On success each argument refers to a linked list of @c dhcp_opt_list nodes, which may be
 *       empty. This interface does not state how such a list ends, so a caller cannot conclude
 *       from a successful return that the last node holds a null @c next, and it does not state
 *       the extent of a node's @c dhcp_opt_val or whether that pointer may be null. On failure it
 *       does not define whether either pointer was written, so a caller must not read or free
 *       either one.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the DHCPv4 option lists as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @warning This interface does not state which side releases the list nodes or the values they
 *          point at, and it declares no matching free function, so a caller can neither release
 *          the result safely nor assume the implementation will. Nor does it state how a list
 *          ends, whether a node's value pointer may be null, or how many bytes a value holds, so
 *          it specifies no safe traversal of the result either: a caller settles the termination
 *          rule, the value representation and the release rule with the implementation it runs
 *          against before it reads or frees anything reached through these two pointers.
 *          @c dhcp_opt_list documents each member on those terms.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetDhcpv6_Options
 * @see dhcp_opt_list
 */
INT platform_hal_GetDhcpv4_Options(dhcp_opt_list **req_opt_list, dhcp_opt_list **send_opt_list);

/**
 * @brief Reads the DHCPv6 options the platform requests and sends.
 *
 * Builds two lists for the DHCPv6 client: the options the platform asks the server for, and the
 * options the platform includes in its own messages.
 *
 * @param[out] req_opt_list - Address of a caller-held @c dhcp_opt_list pointer. The
 *                            implementation builds the list of requested DHCPv6 options and writes
 *                            its head here. The codes it carries are DHCPv6 codes, for which the
 *                            header defines the @c DHCPV6_OPT_ constants; @c dhcp_opt_list states
 *                            what the member does and does not restrict.
 * @param[out] send_opt_list - Address of a caller-held @c dhcp_opt_list pointer. The
 *                             implementation builds the list of DHCPv6 options to be sent and
 *                             writes its head here, on the same terms as @c req_opt_list.
 *
 * @pre Both arguments must be addresses of valid pointer variables. This interface does not define
 *      the behaviour for a null argument.
 * @post On success each argument refers to a linked list of @c dhcp_opt_list nodes, which may be
 *       empty. This interface does not state how such a list ends, so a caller cannot conclude
 *       from a successful return that the last node holds a null @c next, and it does not state
 *       the extent of a node's @c dhcp_opt_val or whether that pointer may be null. On failure it
 *       does not define whether either pointer was written, so a caller must not read or free
 *       either one.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the DHCPv6 option lists as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @warning This interface does not state which side releases the list nodes or the values they
 *          point at, and it declares no matching free function, so a caller can neither release
 *          the result safely nor assume the implementation will. Nor does it state how a list
 *          ends, whether a node's value pointer may be null, or how many bytes a value holds, so
 *          it specifies no safe traversal of the result either: a caller settles the termination
 *          rule, the value representation and the release rule with the implementation it runs
 *          against before it reads or frees anything reached through these two pointers.
 *          @c dhcp_opt_list documents each member on those terms.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see platform_hal_GetDhcpv4_Options
 * @see dhcp_opt_list
 */
INT platform_hal_GetDhcpv6_Options ( dhcp_opt_list **req_opt_list, dhcp_opt_list **send_opt_list);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @enum PSM_STATE
 * @brief Power-saving mode states a platform can report or be placed into.
 *
 * Identifies the power source or thermal condition that governs the platform's
 * power-saving behaviour. A value of this type is passed to
 * @c platform_hal_SetLowPowerModeState through a @c PPSM_STATE pointer.
 *
 * @note These values are data-model state only. Reporting or setting one does not make this
 *       HAL a participant in power management; see @ref platform_hal.
 */
typedef  enum {
   PSM_UNKNOWN=0, /*!< Unknown or unspecified PSM state. */
   PSM_AC, /*!< PSM state when the device is powered by an AC source. */
   PSM_BATT, /*!< PSM state when the device is powered by a battery source. */
   PSM_HOT, /*!< Hot PSM state, possibly indicating a high-temperature condition. */
   PSM_COOLED, /*!< Cooled PSM state, possibly indicating a temperature-regulated condition */
   PSM_NOT_SUPPORTED, /*!< State indicating that the specific PSM is not supported */
} PSM_STATE, *PPSM_STATE;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Sets the platform's power-saving mode state.
 *
 * Records which power-saving state the platform should adopt. Hysteresis is not accounted for
 * before the call, so a caller that toggles rapidly between states may cause the platform to
 * follow each change.
 *
 * @param[in] pState - Caller-allocated, caller-owned @c PSM_STATE holding the state to apply. The
 *                     value must be one of the @c PSM_STATE enumerators. Despite being a pointer, this is an input:
 *                     the implementation reads the state through it and does not write it back. This interface does
 *                     not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pState must point to a @c PSM_STATE holding one of the enumerated values. This interface
 *      does not define the behaviour for a null pointer or an unenumerated value.
 * @post On success the platform has adopted the requested state. On failure this interface does
 *       not define whether the previous state was retained.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the power-saving state as
 *                    indeterminate; this interface declares no getter to read it back.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note This sets a data-model value. It does not make this HAL a participant in power
 *       management, which remains outside its responsibilities; see @ref platform_hal.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see PSM_STATE
 */
INT platform_hal_SetLowPowerModeState(PPSM_STATE pState);

/**
 * @brief Reads the MAC address of the CMTS the device is attached to.
 *
 * Returns the address of the Cable Modem Termination System serving this device, which identifies
 * the head-end the modem is registered with.
 *
 * @param[out] pValue - Caller-allocated, caller-owned buffer receiving the CMTS MAC address as text.
 *                      The buffer must be at least 100 bytes. This interface states neither how many
 *                      bytes the implementation writes nor whether it writes a terminator, so a caller
 *                      must bound every read to the 100 bytes it supplied and, where a terminated
 *                      string is required, must do so itself: zero-filling alone leaves no terminator
 *                      if the implementation writes the whole buffer, so allocate one byte beyond the
 *                      size this interface requires and zero that byte before the call. This interface
 *                      does not specify whether the implementation keeps the pointer after the call
 *                      returns.
 *
 * @pre @c pValue must point to a caller-allocated buffer of at least 100 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the CMTS MAC address as text. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the CMTS MAC address as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c pValue but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_getCMTSMac(CHAR *pValue);


/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @enum WAN_INTERFACE
 * @brief WAN provisioning modes that a DSCP traffic-counting request can target.
 *
 * Selects the WAN interface a DSCP traffic-counting operation applies to. A value of this
 * type is passed to @c platform_hal_setDscp, @c platform_hal_resetDscpCounts and
 * @c platform_hal_getDscpClientList.
 */
typedef enum {
   DOCSIS=1, /*!< DOCSIS mode will be provisioned*/
   EWAN=2,  /*!< EthWAN mode will be provisioned*/
} WAN_INTERFACE;

/**
 * @enum TRAFFIC_CNT_COMMAND
 * @brief Start/stop selector for DSCP traffic counting.
 *
 * Tells @c platform_hal_setDscp whether to begin or to end traffic counting for the DSCP
 * values supplied with the same call.
 */
typedef enum {
   TRAFFIC_CNT_START=1, /*!< Traffic count start */
   TRAFFIC_CNT_STOP=2,  /*!< Traffic count stop */
} TRAFFIC_CNT_COMMAND;

/**
 * @struct Traffic_client
 * @brief Byte counters accumulated for one client against a single DSCP value.
 *
 * One entry of the @c DSCP_Element_t client array. The HAL populates these fields; a caller
 * reads them after @c platform_hal_getDscpClientList has returned successfully.
 */
typedef struct Traffic_client {
    char mac[18];    /*!< The client's MAC address as text in colon-separated upper-case hexadecimal form, for example "00:AA:BB:CC:DD:EE". The field is 18 bytes, which is the 17 characters of that form plus one further byte, so it has no room for any longer representation. This interface states neither how many of those bytes the implementation writes nor whether it writes a terminator, so a caller must bound every read of this field to its 18 bytes and must not rely on a terminator to find the end of the address. */
    ULONG rxBytes;   /*!< The total number of bytes received by the client on this DSCP value since counting started. The domain is that of @c ULONG, which this header defines as @c unsigned long, so its width follows the build target rather than being fixed: 0 to 4,294,967,295 (2^32-1) where @c unsigned long is 32 bits, as it is on the 32-bit RDK-B targets, and 0 to 18,446,744,073,709,551,615 (2^64-1) where it is 64 bits. A caller must derive its own limit from @c unsigned long on the build target, for example from @c ULONG_MAX, rather than assume 64 bits, and on a 32-bit build must sample often enough to keep up with the much shorter span. This interface does not define whether the count wraps or stops at that maximum. */
    ULONG txBytes;   /*!< The total number of bytes transmitted by the client on this DSCP value since counting started. Its domain, its target-dependent width and its undefined behaviour at the maximum are those described for @c rxBytes. */
} Traffic_client_t, *pTraffic_client_t;

/**
 * @struct DSCP_Element
 * @brief Traffic counters for one DSCP value together with the clients observed on it.
 *
 * One entry of the @c DSCP_list_t element array. It carries the DSCP value, the number of
 * client entries the HAL populated, and the fixed-capacity client array itself. Only the
 * first @c numClients entries of @c Client hold meaningful data.
 */
typedef struct DSCP_Element {
    UINT dscp_value;                /*!< The DSCP value these counters were accumulated for. DSCP is a six-bit field, so the value lies in the range 0 to 63 inclusive, as this header also states for the @c dscp_value member of @c hal_network_params_t. The @c UINT type is wider than the field it carries; no value above 63 is meaningful, and this interface does not define what a larger value would mean. */
    UINT numClients;                /*!< The number of leading entries of @c Client the HAL populated. It is a count, not an index: it lies in the range 0 to 256 inclusive, 256 being the capacity of @c Client, and 0 means no client was observed on this DSCP value. The valid indices into @c Client are therefore 0 to @c numClients-1, and at most 0 to 255. */
    Traffic_client_t Client[256];   /*!< Fixed-capacity array of the clients observed on this DSCP value, holding at most 256 entries and indexed 0 to 255. The caller allocates it as part of this structure; the HAL fills the first @c numClients entries, and the remaining entries hold no meaningful data. */
} DSCP_Element_t, *pDSCP_Element_t;

/**
 * @struct DSCP_list
 * @brief Complete set of DSCP traffic-counting results for one WAN interface.
 *
 * The output structure of @c platform_hal_getDscpClientList. The caller allocates the whole
 * structure, including both fixed-capacity arrays, and the HAL fills @c numElements and the
 * leading entries of @c DSCP_Element. Only the first @c numElements entries hold meaningful
 * data.
 */
typedef struct DSCP_list {
    UINT numElements;                  /*!< The number of leading entries of @c DSCP_Element the HAL populated. It is a count, not an index: it lies in the range 0 to 64 inclusive, 64 being the capacity of @c DSCP_Element, and 0 means no DSCP value carried traffic. The valid indices into @c DSCP_Element are therefore 0 to @c numElements-1, and at most 0 to 63. */
    DSCP_Element_t DSCP_Element[64];   /*!< Fixed-capacity array of the DSCP values counted on this WAN interface, holding at most 64 entries and indexed 0 to 63. The caller allocates it as part of this structure; the HAL fills the first @c numElements entries, and the remaining entries hold no meaningful data. */
} DSCP_list_t, *pDSCP_list_t;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Starts or stops per-client traffic counting for a set of DSCP values.
 *
 * Turns DSCP traffic accounting on or off for one WAN interface, over either a named set of DSCP
 * values or all of them.
 *
 * @param[in] interfaceType - WAN interface the request applies to, passed by value. The value must
 *                            be one of the @c WAN_INTERFACE enumerators.
 * @param[in] cmd - Whether to start or stop counting, passed by value. The value must be one of
 *                  the @c TRAFFIC_CNT_COMMAND enumerators.
 * @param[in] pDscpVals - Caller-allocated, caller-owned buffer holding a comma-separated list of
 *                        base-10 DSCP values, for example "10,0", which the caller must supply as a
 *                        zero-terminated string; termination is the caller's obligation on the value it
 *                        passes in. DSCP is a six-bit field, so each value lies in the range 0 to 63
 *                        inclusive. The string must not exceed 195 bytes. Pass NULL to apply the request to
 *                        all DSCP values. The buffer is read during the call; this interface does not specify
 *                        whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pDscpVals must be either NULL or a zero-terminated string of no more than 195 bytes.
 *      This interface does not define the behaviour for an over-length string, a malformed list,
 *      or an @c interfaceType or @c cmd outside its enumeration.
 * @post On success counting is running or stopped as requested for the named values. On failure
 *       this interface does not define whether the request was applied in part.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should not assume counting started, and
 *                    should treat any counters read afterwards as unreliable.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see WAN_INTERFACE
 * @see TRAFFIC_CNT_COMMAND
 * @see platform_hal_getDscpClientList
 * @see platform_hal_resetDscpCounts
 */
INT platform_hal_setDscp(WAN_INTERFACE interfaceType , TRAFFIC_CNT_COMMAND cmd , char *pDscpVals);

/**
 * @brief Resets the DSCP traffic counters for a WAN interface.
 *
 * Returns the accumulated per-client DSCP counters to zero for one WAN interface, which a caller
 * does before starting a fresh measurement interval.
 *
 * @param[in] interfaceType - WAN interface whose counters are to be reset, passed by value. The
 *                            value must be one of the @c WAN_INTERFACE enumerators.
 *
 * @pre None established by this interface. This interface does not define the behaviour for an
 *      @c interfaceType outside its enumeration, nor whether the counters must be running for the
 *      reset to take effect.
 * @post On success the counters for that interface read zero. On failure this interface does not
 *       define whether any counter was reset, so a caller must not assume a clean baseline.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should discard the measurement interval
 *                    rather than report figures taken from an unknown baseline.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see WAN_INTERFACE
 * @see platform_hal_setDscp
 * @see platform_hal_getDscpClientList
 */
INT platform_hal_resetDscpCounts(WAN_INTERFACE interfaceType);


/**
 * @brief Reads the accumulated DSCP traffic counters for a WAN interface.
 *
 * Reports, per DSCP value, the clients seen and the bytes each of them sent and received.
 *
 * @param[in] interfaceType - WAN interface to report on, passed by value. The value must be one of
 *                            the @c WAN_INTERFACE enumerators.
 * @param[out] pDSCP_List - Caller-allocated, caller-owned @c DSCP_list_t structure
 *                          that the implementation fills. The caller allocates the whole structure,
 *                          including its fixed-capacity arrays, so this argument is an out parameter of
 *                          pointer type rather than a pointer to a pointer. This interface does not specify
 *                          whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pDSCP_List must point to a caller-allocated @c DSCP_list_t. This interface does not
 *      define the behaviour for a null pointer or an @c interfaceType outside its enumeration.
 * @post On success @c numElements gives the number of populated entries and only that many leading
 *       entries of @c DSCP_Element hold meaningful data; the same applies to @c numClients within
 *       each element. On failure this interface does not define the structure's contents.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the counter data as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see WAN_INTERFACE
 * @see DSCP_list_t
 * @see platform_hal_setDscp
 */
INT platform_hal_getDscpClientList(WAN_INTERFACE interfaceType , pDSCP_list_t pDSCP_List);


/**
 * @brief Reads the processor speed in bogomips.
 *
 * Returns the kernel's bogomips figure for the processor as decimal text rather than as a number,
 * so a caller that needs an integer must convert it.
 *
 * @param[out] cpuSpeed - Caller-allocated, caller-owned buffer receiving the speed value as decimal text.
 *                        The value is vendor specific and lies in the range 0 to (2^64)-1 inclusive. The
 *                        buffer must be at least 64 bytes. This interface states neither how many bytes
 *                        the implementation writes nor whether it writes a terminator, so a caller must
 *                        bound every read to the 64 bytes it supplied and, where a terminated string is
 *                        required, must do so itself: zero-filling alone leaves no terminator if the
 *                        implementation writes the whole buffer, so allocate one byte beyond the size
 *                        this interface requires and zero that byte before the call. This interface does
 *                        not specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c cpuSpeed must point to a caller-allocated buffer of at least 64 bytes. This interface does
 *      not define the behaviour for a null or undersized buffer, so a caller must not rely
 *      on either being detected.
 * @post On success the output holds the processor speed as decimal text. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the processor speed as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @warning This interface states a minimum size for @c cpuSpeed but defines no length argument
 *          and no truncation behaviour for this function, so a caller that supplies less
 *          than the documented minimum has no defined protection against an overrun.
 */
INT platform_hal_GetCPUSpeed(char *cpuSpeed);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @enum FW_BANK
 * @brief Firmware bank selector.
 *
 * Identifies which firmware bank @c platform_hal_GetFirmwareBankInfo should report on: the
 * bank the device is currently running from, or the alternate bank.
 */
typedef  enum {
   ACTIVE_BANK, /*!< Active firmware bank state */
   INACTIVE_BANK, /*!< Inactive firmware bank state */
} FW_BANK;

/**
 * @struct _FW_BANK_INFO
 * @brief Firmware image name and trial-boot state for one firmware bank.
 *
 * The output structure of @c platform_hal_GetFirmwareBankInfo. The caller allocates it and the
 * implementation fills both fields with text. This interface states neither how many bytes of
 * either field the implementation writes nor whether it writes a terminator, so a caller must
 * bound its reads to @c FW_NAME_MAX_LEN bytes for @c fw_name and @c FW_STATE_MAX_LEN bytes for
 * @c fw_state, and must not rely on a terminator to find the end of either value.
 */
typedef  struct _FW_BANK_INFO
{
      CHAR  fw_name[FW_NAME_MAX_LEN]; /*!< Firmware name. Vendor specific. The buffer size should be at least FW_NAME_MAX_LEN bytes long */
      CHAR  fw_state[FW_STATE_MAX_LEN]; /*!< Firmware state information, Expected values are confirmed, from `TrialBoot#0` to `TrialBoot#n`. TrialBoot value can vary from 0 to n, where "n" is vendor specific. The buffer size should be at least FW_STATE_MAX_LEN bytes long. */
}
FW_BANK_INFO, *PFW_BANK_INFO;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Reads the firmware image name and state held in a given bank.
 *
 * Reports which image occupies the selected firmware bank and what trial-boot state it is in,
 * which a caller uses to confirm an upgrade took hold.
 *
 * @param[in] bankIndex - Bank to report on, passed by value. The value must be one of the
 *                        @c FW_BANK enumerators.
 * @param[out] pFW_Bankinfo - Caller-allocated, caller-owned @c FW_BANK_INFO
 *                            structure that the implementation fills with the image name and state. The
 *                            caller allocates the structure itself, so this argument is an out parameter
 *                            of pointer type rather than a pointer to a pointer. This interface does not
 *                            specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c pFW_Bankinfo must point to a caller-allocated @c FW_BANK_INFO. This interface does not
 *      define the behaviour for a null pointer or a @c bankIndex outside its enumeration.
 * @post On success the output holds the image name in @c fw_name and the trial-boot state in
 *       @c fw_state, each as text of unstated length within its field; this interface does not state
 *       whether either field is terminated, so a caller must bound its reads to @c FW_NAME_MAX_LEN
 *       and @c FW_STATE_MAX_LEN bytes respectively. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the bank information as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see FW_BANK
 * @see FW_BANK_INFO
 */
INT platform_hal_GetFirmwareBankInfo(FW_BANK bankIndex,PFW_BANK_INFO pFW_Bankinfo);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @struct _INTF_STATS
 * @brief Packet and byte counters for one network interface.
 *
 * The output structure of @c platform_hal_GetInterfaceStats. The counters cover LAN-to-WAN
 * and WAN-to-LAN traffic only.
 *
 * @note This interface does not specify the point from which the counters accumulate or
 *       whether anything resets them, so a caller that needs a rate should difference two
 *       reads rather than assume a starting value.
 */
typedef  struct _INTF_STATS
{
   UINT64_t rx_packet; /*!< Packets received. The range of acceptable values is 0 to 2^64 - 1. */
   UINT64_t tx_packet; /*!< Packets sent. The range of acceptable values is 0 to 2^64 - 1. */
   UINT64_t rx_bytes;  /*!< Bytes received. The range of acceptable values is 0 to 2^64 - 1 */
   UINT64_t tx_bytes;  /*!< Bytes sent. The range of acceptable values is 0 to 2^64 - 1. */
}
INTF_STATS, *PINTF_STATS;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Reads packet and byte counters for a named network interface.
 *
 * Reports the traffic counters for one interface, counting LAN-to-WAN and WAN-to-LAN traffic only.
 *
 * @param[in] ifname - Caller-allocated, caller-owned buffer holding the interface name, which the
 *                     caller must supply as a zero-terminated string; termination is the caller's obligation
 *                     on the value it passes in. Interface names are platform specific; 'ifconfig -a' lists
 *                     those a given platform provides. The buffer is read during the call and is declared
 *                     const, so the implementation does not modify it; this interface does not specify
 *                     whether it keeps the pointer after the call returns.
 * @param[out] pIntfStats - Caller-allocated, caller-owned @c INTF_STATS
 *                          structure that the implementation fills with the counters. The caller
 *                          allocates the structure itself, so this argument is an out parameter of
 *                          pointer type rather than a pointer to a pointer. This interface does not
 *                          specify whether the implementation keeps the pointer after the call returns.
 *
 * @pre @c ifname must name an interface that exists on the platform and @c pIntfStats must point
 *      to a caller-allocated @c INTF_STATS. This interface does not define the behaviour for a
 *      null argument or an unknown interface name.
 * @post On success the output holds the four counters for that interface. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if successful.
 * @retval RETURN_ERR if any error is detected. The caller should treat the counters as unavailable,
 *                    discard the output, and retry or report the failure; no more specific
 *                    cause is reported.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @note The counters are cumulative and this interface does not state the point they accumulate
 *       from, so a caller that needs a rate should difference two reads.
 * @see INTF_STATS
 */
INT platform_hal_GetInterfaceStats(const char *ifname,PINTF_STATS pIntfStats);

/**
 * @brief Reads the PPP username configured for the WAN connection.
 *
 * Returns the username the platform authenticates its PPP session with.
 *
 * @param[out] pUserName - Caller-allocated, caller-owned buffer receiving the username as text.
 *                      This interface states neither how many bytes the implementation writes nor
 *                      whether it writes a terminator, so a caller must bound every read to the
 *                      @c maxSize bytes it reported and, where a terminated string is required,
 *                      must do so itself: zero-filling alone leaves no terminator if the
 *                      implementation writes the whole buffer, so report a @c maxSize one byte
 *                      short of the allocation and zero the byte beyond it before the call. This
 *                      interface does not specify whether the implementation keeps the pointer
 *                      after the call returns.
 * @param[in] maxSize - Size in bytes of the buffer supplied in @c pUserName, passed by value.
 *
 * @pre @c pUserName must point to a caller-allocated buffer of at least @c maxSize bytes. This
 *      interface does not define the behaviour for a null pointer or for a @c maxSize that
 *      misreports the allocation.
 * @post On success the output holds the requested value. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the username is successfully retrieved.
 * @retval RETURN_ERR if an error is encountered during the operation. The caller should treat
 *                    the username as unavailable and must not fall back to a default or an empty
 *                    string.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.

 * @warning The @c maxSize argument is the only bound the implementation has; a caller that
 *          reports more than it allocated has no defined protection against an overrun.
 * @see platform_hal_GetPppPassword
 */
INT platform_hal_GetPppUserName(CHAR* pUserName, ULONG maxSize);

/**
 * @brief Reads the PPP password configured for the WAN connection.
 *
 * Returns the password the platform authenticates its PPP session with.
 *
 * @param[out] pPassword - Caller-allocated, caller-owned buffer receiving the password as text.
 *                      This interface states neither how many bytes the implementation writes nor
 *                      whether it writes a terminator, so a caller must bound every read to the
 *                      @c maxSize bytes it reported and, where a terminated string is required,
 *                      must do so itself: zero-filling alone leaves no terminator if the
 *                      implementation writes the whole buffer, so report a @c maxSize one byte
 *                      short of the allocation and zero the byte beyond it before the call. This
 *                      interface does not specify whether the implementation keeps the pointer
 *                      after the call returns.
 * @param[in] maxSize - Size in bytes of the buffer supplied in @c pPassword, passed by value.
 *
 * @pre @c pPassword must point to a caller-allocated buffer of at least @c maxSize bytes. This
 *      interface does not define the behaviour for a null pointer or for a @c maxSize that
 *      misreports the allocation.
 * @post On success the output holds the requested value. On failure this interface does not define the
 *       output's contents, so a caller must discard them.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the password is successfully retrieved.
 * @retval RETURN_ERR if an error is encountered during the operation. The caller should treat
 *                    the password as unavailable and must not fall back to a default or an empty
 *                    string.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.

 * @warning The value returned is a credential. A caller should keep it out of logs and
 *          diagnostics and clear the buffer once it is no longer needed. The @c maxSize argument
 *          is the only bound the implementation has; a caller that reports more than it allocated
 *          has no defined protection against an overrun.
 * @see platform_hal_GetPppUserName
 */
INT platform_hal_GetPppPassword(CHAR* pPassword, ULONG maxSize);

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_TYPES
 * @{
 */

/**
 * @enum ip_version_t
 * @brief Enumeration of IP version types.
 *
 * This enum defines the supported IP versions for the network parameters,
 * including IPv4 and IPv6.
 */
typedef enum {
    IP_VERSION_IPV4,  /**< IPv4 */
    IP_VERSION_IPV6   /**< IPv6 */
} ip_version_t;

/**
 * @brief Enumeration of transport protocol types.
 *
 * This enum defines the supported transport protocols for network traffic,
 * including Transmission Control Protocol (TCP) and User Datagram Protocol (UDP).
 */
typedef enum {
    PROTOCOL_TCP,  /**< Transmission Control Protocol (TCP) */
    PROTOCOL_UDP   /**< User Datagram Protocol (UDP) */
} net_proto_t;

/**
 * @brief Structure representing network parameters for QoS and traffic management.
 *
 * This structure carries the network settings for managing Quality of Service (QoS) and traffic: a source
 * MAC address, an IP version selector, source and destination IP addresses, source and destination port
 * numbers, a transport protocol and a DSCP value. It declares no destination MAC address member.
 */
typedef struct {
    /**
     * @brief Source MAC Address.
     *
     * The MAC address of the source device, stored as a 6-byte array 
     * in standard hardware address format.
     */
    UINT8_t src_mac[6]; /**< Source MAC Address. */

    /**
     * @brief IP version.
     *
     * Indicates the IP version being used, either IPv4 or IPv6. 
     * This field determines the format of the `src_ip` and `dest_ip` fields.
     */
    ip_version_t ip_version; /**< IP version (IPv4 or IPv6). */

    /**
     * @brief Source IP Address.
     *
     * Union that stores the source IP address, which can be in either 
     * IPv4 or IPv6 format. The specific member used is determined by 
     * the value of the `ip_version` field.
     */
    union {
        UINT32_t ipv4;          /**< Source IP Address in IPv4 format (32 bits). */
        UINT8_t ipv6[16];       /**< Source IP Address in IPv6 format (128 bits). */
    } src_ip;

    /**
     * @brief Destination IP Address.
     *
     * Union that stores the destination IP address, which can be in 
     * either IPv4 or IPv6 format. The specific member used is determined 
     * by the `ip_version` field value.
     */
    union {
        UINT32_t ipv4;          /**< Destination IP Address in IPv4 format (32 bits). */
        UINT8_t ipv6[16];       /**< Destination IP Address in IPv6 format (128 bits). */
    } dest_ip;

    /**
     * @brief Destination Port.
     *
     * The port number for the destination endpoint is specified in 
     * network byte order. This field is used to identify the target 
     * service or application on the destination device.
     *
     * @note Valid range: 0 to 65535.
     */
    UINT16_t dest_port; /**< Destination Port number in network byte order. */

    /**
     * @brief Source Port.
     *
     * The port number for the source endpoint is specified in network byte order. 
     * This field is used to identify the originating service or application on 
     * the source device.
     *
     * @note Valid range: 0 to 65535.
     */
    UINT16_t src_port; /**< Source Port number in network byte order. */

    /**
     * @brief Transport protocol.
     *
     * Specifies the transport layer protocol used for the network communication, 
     * such as TCP or UDP.
     */
    net_proto_t protocol; /**< Transport protocol (TCP or UDP). */

    /**
     * @brief Differentiated Services Code Point (DSCP) Value for QoS.
     *
     * This field specifies the DSCP value used in the IP header for QoS marking. 
     * The DSCP value is 6 bits wide and ranges from 0 to 63. It is used to 
     * prioritize network traffic for better Quality of Service.
     */
    UINT8_t dscp_value; /**< DSCP value for QoS (range: 0-63). */
} hal_network_params_t;

/**
 * @}
 */

/**
 * @addtogroup PLATFORM_HAL_APIS
 * @{
 */

/**
 * @brief Applies QoS and traffic management settings from a network parameter set.
 *
 * Programs a Quality of Service and traffic management rule in the platform from a single
 * caller-supplied descriptor covering the flow's addresses, ports, protocol and DSCP marking.
 *
 * @param[in] params - Caller-allocated, caller-owned @c hal_network_params_t structure holding
 *                     the rule to apply. Every member and its valid range is described on @c hal_network_params_t;
 *                     the address union in use is selected by the @c ip_version member. The structure is declared
 *                     const and is read during the call, so the implementation does not modify it; this
 *                     interface does not specify whether it keeps the pointer after the call returns.
 *
 * @pre @c params must point to a fully initialised @c hal_network_params_t whose @c ip_version
 *      matches the address union member the caller populated. This interface does not define the
 *      behaviour for a null pointer, a partially initialised structure, or a member outside its
 *      documented range.
 * @post On success the rule is in force. On failure this interface does not define whether the
 *       rule was applied in part, so a caller should not assume the platform is unchanged.
 *
 * @returns The status of the operation.
 * @retval RETURN_OK if the QoS and traffic management settings were applied. Its value is 0.
 * @retval RETURN_ERR if the settings could not be applied. Its value is -1, and this interface
 *                    defines no other negative code for this function. The caller should report
 *                    the failure rather than assume the rule is active.
 *
 * @execution Synchronous; the calling thread is blocked until the operation completes. This
 *            interface publishes no numeric time bound for this function.
 * @note Not thread safe. A caller must serialise this call against every other Platform HAL
 *       call it makes.
 * @see hal_network_params_t
 * @see ip_version_t
 * @see net_proto_t
 */
INT platform_hal_qos_apply(const hal_network_params_t *params);

#ifdef __cplusplus
}
#endif

#endif

/**
 * @}
 */


