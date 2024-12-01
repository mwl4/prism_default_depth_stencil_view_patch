#include <Windows.h>
#include <psapi.h>

#include "polyhook2/Misc.hpp"

namespace prism_default_depth_stencil_view_patch
{

const char *const PATCH_VERSION =                   "1.53";

constexpr uint32_t SCS_SDK_supported_version =      ( 1 << 16 ) | 1;

constexpr int32_t SCS_SDK_LOG_message =             0;
constexpr int32_t SCS_SDK_LOG_warning =             1;
constexpr int32_t SCS_SDK_LOG_error =               2;

using scs_sdk_log_t =                               void( * )( const int32_t type, const char *message );

struct scs_sdk_params_t
{
    char                                            padding[ 24 ];
    scs_sdk_log_t                                   log;
};

constexpr int32_t SCS_SDK_RESULT_ok =               0;
constexpr int32_t SCS_SDK_RESULT_unsupported =      -1;
constexpr int32_t SCS_SDK_RESULT_invalid_param =    -2;
constexpr int32_t SCS_SDK_RESULT_generic_error =    -7;

class logger_t
{
public:
    logger_t() {}

    logger_t( const logger_t & ) = delete;
    logger_t &operator=( const logger_t & ) = delete;

    void init( scs_sdk_log_t log_fptr )
    {
        assert( m_initialized == false ); // init can be called only once
        m_log_fptr = log_fptr;
        m_initialized = true;
    }

    void destroy()
    {
        m_initialized = false;
        m_log_fptr = nullptr;
    }

    void message( const char *format, ... )
    {
        va_list va;
        va_start( va, format );
        log_tagged( SCS_SDK_LOG_message, format, va );
        va_end( va );
    }

    void warning( const char *format, ... )
    {
        va_list va;
        va_start( va, format );
        log_tagged( SCS_SDK_LOG_warning, format, va );
        va_end( va );
    }

    void error( const char *format, ... )
    {
        va_list va;
        va_start( va, format );
        log_tagged( SCS_SDK_LOG_error, format, va );
        va_end( va );
    }

private:
    void log_tagged( const int32_t type, const char *format, va_list va )
    {
        if( m_log_fptr )
        {
            char formatted[ 1024 ];
            vsprintf_s( formatted, format, va );
            char tagged_message[ 1024 ];
            sprintf_s( tagged_message, "[prism_default_depth_stencil_view_patch] %s", formatted );
            m_log_fptr( type, tagged_message );
        }
    }

private:
    bool m_initialized = false;
    scs_sdk_log_t m_log_fptr = nullptr;
};

logger_t log;

class exception_local_string_t : public std::exception
{
public:
    exception_local_string_t() {}

    explicit exception_local_string_t( const char *format, ... )
    {
        va_list va;
        va_start( va, format );
        vsprintf_s( m_message, format, va );
        va_end( va );
    }

    static exception_local_string_t with_last_error( const char *format, ... )
    {
        const DWORD last_error = GetLastError(); // capture it first, before anything else
        char message[ 1024 ];
        va_list va;
        va_start( va, format );
        vsprintf_s( message, format, va );
        va_end( va );
        exception_local_string_t result;
        sprintf_s( result.m_message, "%s; err: %lu", message, last_error );
        return result;
    }

    virtual char const *what() const override
    {
        return m_message;
    }

    char m_message[ 1024 ] = {};
};

class memory_protect_scope_t
{
public:
    memory_protect_scope_t( void *address, size_t size, DWORD protect )
        : m_address( address )
        , m_size( size )
    {
        if( VirtualProtect( address, size, protect, &m_old_protect ) == FALSE )
        {
            throw exception_local_string_t::with_last_error( "Unable to protect memory to target state (address: %p, size: %d, protect: %lu)", address, int( size ), protect );
        }
    }

    ~memory_protect_scope_t()
    {
        DWORD old_protect = 0;
        if( VirtualProtect( m_address, m_size, m_old_protect, &old_protect ) == FALSE )
        {
            // you may think it is good idea to throw exception here, but it is not in case of destructor.
            fprintf( stderr, "Unable to protect memory back to its previous state (address: %p, size: %d, protect: %lu)", m_address, int( m_size ), m_old_protect );
        }
    }

    memory_protect_scope_t( const memory_protect_scope_t & ) = delete;
    memory_protect_scope_t &operator=( const memory_protect_scope_t & ) = delete;

private:
    void *m_address = nullptr;
    size_t m_size = 0;
    DWORD m_old_protect = 0;
};

void apply_patch()
{
    const HMODULE module = GetModuleHandleA( NULL );
    if( module == NULL )
    {
        throw exception_local_string_t::with_last_error( "Unable to get module handle" );
    }

    MODULEINFO module_info = {};
    if( GetModuleInformation( GetCurrentProcess(), module, &module_info, sizeof( module_info ) ) == FALSE )
    {
        throw exception_local_string_t::with_last_error( "Unable to get module information" );
    }

    const uintptr_t range_start = reinterpret_cast< uintptr_t >( module );
    const uintptr_t range_end = range_start + module_info.SizeOfImage;

    const char *const pattern =
        /* test    al, al           */ "84 C0 "
        /* jz      short loc_xxxx   */ "74 ?? "
        /* cmp     r8d, [rbx+xxh]   */ "44 3B 43 ?? "
        /* jnz     short loc_xxxx   */ "75 ?? "
        /* cmp     [rbx+xxh], cl    */ "38 4B ?? "
        /* jz      short loc_xxxx   */ "74 ??";

    const uintptr_t code_to_patch_address = PLH::findPattern( range_start, range_end - range_start, pattern );

    if( code_to_patch_address == NULL )
    {
        throw exception_local_string_t( "Unable to find code to patch with pattern" );
    }

    void *const code_to_patch_ptr = reinterpret_cast< void * >( code_to_patch_address );

    const uint8_t skipping_default_path_bytecode[] =
    {
        /* mov    al, 0 */ 0xB0, 0x00,
        /* test   al,al */ 0x84, 0xC0,
        /* nop; nop     */ 0x90, 0x90,
        /* nop; nop     */ 0x90, 0x90,
        /* nop; nop     */ 0x90, 0x90,
        /* nop; nop     */ 0x90, 0x90,
        /* nop          */ 0x90,
    };

    {
        memory_protect_scope_t memory_protect( code_to_patch_ptr, sizeof( skipping_default_path_bytecode ), PAGE_EXECUTE_READWRITE );

        memcpy( code_to_patch_ptr, skipping_default_path_bytecode, sizeof( skipping_default_path_bytecode ) );
    }

    if( memcmp( code_to_patch_ptr, skipping_default_path_bytecode, sizeof( skipping_default_path_bytecode ) ) != 0 )
    {
        throw exception_local_string_t( "Unable to hook game code" );
    }

    log.message( "Successfully patched game code at: %p", code_to_patch_ptr );
}

extern "C" __declspec( dllexport ) int32_t scs_telemetry_init( const uint32_t version, const scs_sdk_params_t *const params )
{
    if( version != SCS_SDK_supported_version )
    {
        return SCS_SDK_RESULT_unsupported;
    }

    if( params == nullptr )
    {
        return SCS_SDK_RESULT_invalid_param;
    }

    if( params->log == nullptr )
    {
        return SCS_SDK_RESULT_invalid_param;
    }

    log.init( params->log );

    static bool already_patched = false;

    if( already_patched )
    {
        log.error( "Game code is already patched." );
        return SCS_SDK_RESULT_ok;
    }

    log.message( "Patcher originally made for game version: %s.", PATCH_VERSION );

    try
    {
        apply_patch();
        already_patched = true;
        return SCS_SDK_RESULT_ok;
    }
    catch( const std::exception &e )
    {
        log.error( "%s.", e.what() );
        return SCS_SDK_RESULT_generic_error;
    }
}

extern "C" __declspec( dllexport ) void scs_telemetry_shutdown()
{
    log.destroy();
}

} // namespace prism_default_depth_stencil_view_patch

BOOL APIENTRY DllMain( HMODULE module, DWORD reason_for_call, LPVOID reseved )
{
    return TRUE;
}

/* eof */
