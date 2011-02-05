#include "npobj.h"

#ifdef __cplusplus
extern "C" {
#endif

extern NPNetscapeFuncs *npnfuncs;
#ifdef __cplusplus
};
#endif


struct NPClass NPObj::_npo_class = {
    NP_CLASS_STRUCT_VERSION,
    NULL,//_allocate,
    _deallocate,
    NULL,
    _hasMethod,
    _invoke,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

NPObj::NPObj( NPP instance, bool retainObject ) 
{
    LOGF;
    _npobject = npnfuncs->createobject( instance, &_npo_class );
    if( retainObject ){
        npnfuncs->retainobject( _npobject );
    }
    _npp = instance;
    NPObj::_map[ _npobject ] = this;
}

NPObj::~NPObj()
{
    LOGF;
    NPObj::_map.erase(_npobject);
    npnfuncs->releaseobject( _npobject );
}

bool NPObj::_hasMethod( NPObject *obj, NPIdentifier methodName )
{
    bool r;
    DWORD w;
    LPWSTR s;
    NPUTF8 *name = npnfuncs->utf8fromidentifier( methodName );

    LOGF;
    const Map::const_iterator it = NPObj::_map.find(obj);
    if (it == NPObj::_map.end()) {
      return false;
    }
    w = MultiByteToWideChar( CP_UTF8, 0, name, -1, NULL, 0 );
    s = new WCHAR[ w + 1 ];
    MultiByteToWideChar( CP_UTF8, 0, name, -1, s, w );
    LOG( L"methodName=%s", s );
    r = it->second->hasMethod( s );

    delete s;
    npnfuncs->memfree( name );
    return r;

}

bool NPObj::_invoke( NPObject *obj, NPIdentifier methodName, 
        const NPVariant *args, uint32_t argCount, NPVariant *result) 
{
    bool r;
    DWORD w;
    LPWSTR s;
    NPUTF8 *name = npnfuncs->utf8fromidentifier( methodName );

    LOGF;
    const Map::const_iterator it = NPObj::_map.find(obj);
    if (it == NPObj::_map.end()) {
      return false;
    }
    w = MultiByteToWideChar( CP_UTF8, 0, name, -1, NULL, 0 );
    s = new WCHAR[ w + 1 ];
    MultiByteToWideChar( CP_UTF8, 0, name, -1, s, w );
    LOG( L"methodName=%s", s );
    r = it->second->invoke( s, args, argCount, result );
    delete s;
    npnfuncs->memfree( name );
    return r;
}

/* 
NPObject* NPObj::_allocate( NPP npp, NPClass *aClass )
{
    NPObj* p;

    LOGF;
    p = new NPObj( npp );
    return p->getNPObject();
}
*/

void NPObj::_deallocate( NPObject *obj )
{
    LOGF;

    const Map::const_iterator it = NPObj::_map.find(obj);
    if (it == NPObj::_map.end()) {
      return;
    }
    delete it->second;
}

NPObject* NPObj::getNPObject()
{
    return _npobject;
}

NPP NPObj::getNPP()
{
    return _npp;
}

#pragma warning( push )
#pragma warning( disable : 4100 )
bool NPObj::hasMethod( LPCWSTR methodName )
{
    LOGF;
    return false;
}
#pragma warning( pop )

#pragma warning( push )
#pragma warning( disable : 4100 )
bool NPObj::invoke( LPCWSTR methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) 
{
    LOGF;
    return false;
}
#pragma warning( pop )

NPObj::Map NPObj::_map;

LPCSTR NPOE_MISSING_ARGUMENT = "missing argument"; 
LPCSTR NPOE_INVALID_ARG_TYPE = "invalid argument type"; 
LPCSTR NPOE_GENERIC_ERROR = "error";

LPCSTR checkNpArgs( LPCSTR lpszCheck, const NPVariant *args, uint32_t argCount )
{
    DWORD i;
    LOGF;
    LOG( "argCount=%d", argCount );
    for( i = 0; i < argCount; i++ ){
        if( !*lpszCheck ) break;
        switch( *lpszCheck ){
        case 'V' : // VOID
            if( !NPVARIANT_IS_VOID( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case 'N' : // NULL
            if( !NPVARIANT_IS_NULL( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case 'B' : // boolean
            if( !NPVARIANT_IS_BOOLEAN( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case 'I' : // INT32
            if( !NPVARIANT_IS_INT32( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case 'D' : // DOUBLE
            if( !NPVARIANT_IS_DOUBLE( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case 'S' : // STRING
            if( !NPVARIANT_IS_STRING( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case 'O' : // OBJECT
            if( !NPVARIANT_IS_OBJECT( args[ i ] ) ) return NPOE_INVALID_ARG_TYPE;
            break;
        case '*' : // anything
        case '?':
            break;
        default:
            return NPOE_GENERIC_ERROR;
        }
        *lpszCheck++;
    }
    if( *lpszCheck ) return NPOE_MISSING_ARGUMENT;
    return NULL;
}

LPWSTR Npv2WStr( NPVariant v )
{
    DWORD w;
    LPWSTR s;

    if( NPVARIANT_IS_NULL( v ) ){
        s = new WCHAR[ 1 ];
        *s = L'\0';
    }else if( NPVARIANT_IS_BOOLEAN( v ) ){
        s = new WCHAR[ 6 ];
        if( NPVARIANT_TO_BOOLEAN( v ) ){
            StringCchCopyW( s, 6, L"true" );
        }else{
            StringCchCopyW( s, 6, L"false" );
        }
    }else if( NPVARIANT_IS_INT32( v ) ){
        s = new WCHAR[ 11 ];
        StringCchPrintfW( s, 11, L"%d", NPVARIANT_TO_INT32( v ) );
    }else if( NPVARIANT_IS_DOUBLE( v ) ){
        s = new WCHAR[ 30 ];
        StringCchPrintfW( s, 11, L"%f", NPVARIANT_TO_DOUBLE( v ) );
    }else if( NPVARIANT_IS_STRING( v ) ){
        NPString str = NPVARIANT_TO_STRING( v );
        w = MultiByteToWideChar( CP_UTF8, 0, str.UTF8Characters, str.UTF8Length + 1, NULL, 0 );
        s = new WCHAR[ w + 1 ];
        MultiByteToWideChar( CP_UTF8, 0, str.UTF8Characters, str.UTF8Length + 1, s, w );
    }else{
        s = new WCHAR[ 1 ];
        *s = L'\0';
    }
    return s;
}

LPSTR Npv2Str( NPVariant v )
{
    DWORD w;
    LPSTR s;
    LPWSTR ws;

    ws = Npv2WStr( v );
    if( ws == NULL ) return NULL;

    w = WideCharToMultiByte( CP_ACP, 0, ws, -1, NULL, 0, NULL, NULL );
    s = new CHAR[ w + 1 ];
    WideCharToMultiByte( CP_ACP, 0, ws, -1, s, w, NULL, NULL );
    delete ws;
    return s;
}

int Npv2Int( NPVariant v )
{
    if( NPVARIANT_IS_NULL( v ) ){
        return 0;
    }else if( NPVARIANT_IS_BOOLEAN( v ) ){
        if( NPVARIANT_TO_BOOLEAN( v ) ){
            return 1;
        }else{
            return 0;
        }
    }else if( NPVARIANT_IS_INT32( v ) ){
        return NPVARIANT_TO_INT32( v );
    }else if( NPVARIANT_IS_STRING( v ) ){
        int n;
        LPCWSTR s = Npv2WStr( v );
        if( StrToIntExW( s, STIF_DEFAULT, &n ) ){
            return n;
        }
    }
    return 0;
}

NPUTF8* allocUtf8( LPCSTR s )
{
    // convert to UTF-16
    DWORD w = MultiByteToWideChar( CP_ACP, 0, s, -1, NULL, 0 );
    LPWSTR sw = new WCHAR[ w + 1 ];
    MultiByteToWideChar( CP_ACP, 0, s, -1, sw, w + 1 );

    // convert to UTF-8
    w = WideCharToMultiByte( CP_UTF8, 0, sw, -1, NULL, 0, 0, 0 );
    NPUTF8 *sa = (NPUTF8 *)(npnfuncs->memalloc( w + 1 ));
    WideCharToMultiByte( CP_UTF8, 0, sw, -1, sa, w + 1, 0, 0 );
    delete sw;
    return sa;
}

NPUTF8* allocUtf8( LPCWSTR s )
{
    // convert to UTF-8
    DWORD w = WideCharToMultiByte( CP_UTF8, 0, s, -1, NULL, 0, 0, 0 );
    NPUTF8 *sa = (NPUTF8 *)(npnfuncs->memalloc( w + 1 ));
    WideCharToMultiByte( CP_UTF8, 0, s, -1, sa, w + 1, 0, 0 );
    return sa;
}
