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
    NPUTF8 *name = npnfuncs->utf8fromidentifier( methodName );

    LOGF;
    const Map::const_iterator it = NPObj::_map.find(obj);
    if (it == NPObj::_map.end()) {
      return false;
    }
    w = MultiByteToWideChar( CP_UTF8, 0, name, -1, NULL, 0 );
    std::vector<wchar_t> s(w+1, '\0');
    MultiByteToWideChar( CP_UTF8, 0, name, -1, s.data(), w );
    LOG( L"methodName=%s", s.data() );
    r = it->second->hasMethod(s.data());
    npnfuncs->memfree( name );
    return r;

}

bool NPObj::_invoke( NPObject *obj, NPIdentifier methodName, 
        const NPVariant *args, uint32_t argCount, NPVariant *result) 
{
    bool r;
    DWORD w;
    NPUTF8 *name = npnfuncs->utf8fromidentifier( methodName );

    LOGF;
    const Map::const_iterator it = NPObj::_map.find(obj);
    if (it == NPObj::_map.end()) {
      return false;
    }
    w = MultiByteToWideChar( CP_UTF8, 0, name, -1, NULL, 0 );
    std::vector<wchar_t> vec(w+1, '\0');
    MultiByteToWideChar( CP_UTF8, 0, name, -1, vec.data(), w );
    LOG( L"methodName=%s", vec.data() );
    r = it->second->invoke( vec.data(), args, argCount, result );
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

std::wstring Npv2WStr( NPVariant v )
{
    if( NPVARIANT_IS_NULL( v ) ){
      return std::wstring();
    }else if( NPVARIANT_IS_BOOLEAN( v ) ){
        if( NPVARIANT_TO_BOOLEAN( v ) ){
          return L"true";
        }else{
          return L"false";
        }
    }else if( NPVARIANT_IS_INT32( v ) ){
        std::vector<wchar_t> vec(11, '\0');
        StringCchPrintfW( vec.data(), 11, L"%d", NPVARIANT_TO_INT32(v) );
        return std::wstring(vec.data());
    }else if( NPVARIANT_IS_DOUBLE( v ) ){
        std::vector<wchar_t> vec(30, '\0');
        StringCchPrintfW( vec.data(), 11, L"%f", NPVARIANT_TO_DOUBLE(v) );
        return std::wstring(vec.data());
    }else if( NPVARIANT_IS_STRING( v ) ){
        NPString str = NPVARIANT_TO_STRING( v );
        DWORD w = MultiByteToWideChar( CP_UTF8, 0, str.UTF8Characters, str.UTF8Length + 1, NULL, 0 );
        std::vector<wchar_t> vec(w+1, '\0');
        MultiByteToWideChar( CP_UTF8, 0, str.UTF8Characters, str.UTF8Length + 1, vec.data(), w );
        return std::wstring(vec.data());
    }else {
        return std::wstring();
    }
}

std::string Npv2Str( NPVariant v )
{
    std::wstring ws = Npv2WStr( v );
    if( ws.empty() ) return std::string();
    const DWORD w = WideCharToMultiByte( CP_ACP, 0, ws.c_str(), -1, NULL, 0, NULL, NULL );
    std::vector<char> vec(w+1, '\0');
    WideCharToMultiByte( CP_ACP, 0, ws.c_str(), -1, vec.data(), w, NULL, NULL );
    return std::string(vec.data());
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
        std::wstring s = Npv2WStr( v );
        if( StrToIntExW( s.c_str(), STIF_DEFAULT, &n ) ){
            return n;
        }
    }
    return 0;
}

NPUTF8* allocUtf8( LPCSTR s )
{
    // convert to UTF-16
    DWORD w = MultiByteToWideChar( CP_ACP, 0, s, -1, NULL, 0 );
    std::vector<wchar_t> sw(w + 1, '\0');
    MultiByteToWideChar( CP_ACP, 0, s, -1, sw.data(), w + 1 );

    // convert to UTF-8
    w = WideCharToMultiByte( CP_UTF8, 0, sw.data(), -1, NULL, 0, 0, 0 );
    NPUTF8 *sa = (NPUTF8 *)(npnfuncs->memalloc( w + 1 ));
    WideCharToMultiByte( CP_UTF8, 0, sw.data(), -1, sa, w + 1, 0, 0 );
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
