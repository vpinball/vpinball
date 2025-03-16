/*
 * Copyright 2011 Jacek Caban for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <assert.h>
#include <limits.h>
#include <math.h>

#include "vbscript.h"
#include "parse.h"
#include "parser.tab.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(vbscript);

static const struct {
    const WCHAR *word;
    int token;
} keywords[] = {
    {L"and",       tAND},
    {L"byref",     tBYREF},
    {L"byval",     tBYVAL},
    {L"call",      tCALL},
    {L"case",      tCASE},
    {L"class",     tCLASS},
    {L"const",     tCONST},
    {L"default",   tDEFAULT},
    {L"dim",       tDIM},
    {L"do",        tDO},
    {L"each",      tEACH},
    {L"else",      tELSE},
    {L"elseif",    tELSEIF},
    {L"empty",     tEMPTY},
    {L"end",       tEND},
    {L"eqv",       tEQV},
    {L"error",     tERROR},
    {L"exit",      tEXIT},
    {L"explicit",  tEXPLICIT},
    {L"false",     tFALSE},
    {L"for",       tFOR},
    {L"function",  tFUNCTION},
    {L"get",       tGET},
    {L"goto",      tGOTO},
    {L"if",        tIF},
    {L"imp",       tIMP},
    {L"in",        tIN},
    {L"is",        tIS},
    {L"let",       tLET},
    {L"loop",      tLOOP},
    {L"me",        tME},
    {L"mod",       tMOD},
    {L"new",       tNEW},
    {L"next",      tNEXT},
    {L"not",       tNOT},
    {L"nothing",   tNOTHING},
    {L"null",      tNULL},
    {L"on",        tON},
    {L"option",    tOPTION},
    {L"or",        tOR},
    {L"preserve",  tPRESERVE},
    {L"private",   tPRIVATE},
    {L"property",  tPROPERTY},
    {L"public",    tPUBLIC},
    {L"redim",     tREDIM},
    {L"rem",       tREM},
    {L"resume",    tRESUME},
    {L"select",    tSELECT},
    {L"set",       tSET},
    {L"step",      tSTEP},
    {L"stop",      tSTOP},
    {L"sub",       tSUB},
    {L"then",      tTHEN},
    {L"to",        tTO},
    {L"true",      tTRUE},
    {L"until",     tUNTIL},
    {L"wend",      tWEND},
    {L"while",     tWHILE},
    {L"with",      tWITH},
    {L"xor",       tXOR}
};

static const double pow10[309] = {
1.e0,1.e1,1.e2,1.e3,1.e4,1.e5,1.e6,1.e7,1.e8,1.e9,1.e10,1.e11,1.e12,1.e13,1.e14,1.e15,1.e16,
1.e17,1.e18,1.e19,1.e20,1.e21,1.e22,1.e23,1.e24,1.e25,1.e26,1.e27,1.e28,1.e29,1.e30,1.e31,
1.e32,1.e33,1.e34,1.e35,1.e36,1.e37,1.e38,1.e39,1.e40,1.e41,1.e42,1.e43,1.e44,1.e45,1.e46,
1.e47,1.e48,1.e49,1.e50,1.e51,1.e52,1.e53,1.e54,1.e55,1.e56,1.e57,1.e58,1.e59,1.e60,1.e61,
1.e62,1.e63,1.e64,1.e65,1.e66,1.e67,1.e68,1.e69,1.e70,1.e71,1.e72,1.e73,1.e74,1.e75,1.e76,
1.e77,1.e78,1.e79,1.e80,1.e81,1.e82,1.e83,1.e84,1.e85,1.e86,1.e87,1.e88,1.e89,1.e90,1.e91,
1.e92,1.e93,1.e94,1.e95,1.e96,1.e97,1.e98,1.e99,1.e100,1.e101,1.e102,1.e103,1.e104,1.e105,
1.e106,1.e107,1.e108,1.e109,1.e110,1.e111,1.e112,1.e113,1.e114,1.e115,1.e116,1.e117,1.e118,
1.e119,1.e120,1.e121,1.e122,1.e123,1.e124,1.e125,1.e126,1.e127,1.e128,1.e129,1.e130,1.e131,
1.e132,1.e133,1.e134,1.e135,1.e136,1.e137,1.e138,1.e139,1.e140,1.e141,1.e142,1.e143,1.e144,
1.e145,1.e146,1.e147,1.e148,1.e149,1.e150,1.e151,1.e152,1.e153,1.e154,1.e155,1.e156,1.e157,
1.e158,1.e159,1.e160,1.e161,1.e162,1.e163,1.e164,1.e165,1.e166,1.e167,1.e168,1.e169,1.e170,
1.e171,1.e172,1.e173,1.e174,1.e175,1.e176,1.e177,1.e178,1.e179,1.e180,1.e181,1.e182,1.e183,
1.e184,1.e185,1.e186,1.e187,1.e188,1.e189,1.e190,1.e191,1.e192,1.e193,1.e194,1.e195,1.e196,
1.e197,1.e198,1.e199,1.e200,1.e201,1.e202,1.e203,1.e204,1.e205,1.e206,1.e207,1.e208,1.e209,
1.e210,1.e211,1.e212,1.e213,1.e214,1.e215,1.e216,1.e217,1.e218,1.e219,1.e220,1.e221,1.e222,
1.e223,1.e224,1.e225,1.e226,1.e227,1.e228,1.e229,1.e230,1.e231,1.e232,1.e233,1.e234,1.e235,
1.e236,1.e237,1.e238,1.e239,1.e240,1.e241,1.e242,1.e243,1.e244,1.e245,1.e246,1.e247,1.e248,
1.e249,1.e250,1.e251,1.e252,1.e253,1.e254,1.e255,1.e256,1.e257,1.e258,1.e259,1.e260,1.e261,
1.e262,1.e263,1.e264,1.e265,1.e266,1.e267,1.e268,1.e269,1.e270,1.e271,1.e272,1.e273,1.e274,
1.e275,1.e276,1.e277,1.e278,1.e279,1.e280,1.e281,1.e282,1.e283,1.e284,1.e285,1.e286,1.e287,
1.e288,1.e289,1.e290,1.e291,1.e292,1.e293,1.e294,1.e295,1.e296,1.e297,1.e298,1.e299,1.e300,
1.e301,1.e302,1.e303,1.e304,1.e305,1.e306,1.e307,1.e308};

static inline BOOL is_identifier_char(WCHAR c)
{
    return iswalnum(c) || c == '_';
}

static int check_keyword(parser_ctx_t *ctx, const WCHAR *word, const WCHAR **lval)
{
    const WCHAR *p1 = ctx->ptr;
    const WCHAR *p2 = word;
    WCHAR c;

    while(p1 < ctx->end && *p2) {
        c = towlower(*p1);
        if(c != *p2)
            return c - *p2;
        p1++;
        p2++;
    }

    if(*p2 || (p1 < ctx->end && is_identifier_char(*p1)))
        return 1;

    ctx->ptr = p1;
    *lval = word;
    return 0;
}

static int check_keywords(parser_ctx_t *ctx, const WCHAR **lval)
{
    int min = 0, max = ARRAY_SIZE(keywords)-1, r, i;

    while(min <= max) {
        i = (min+max)/2;

        r = check_keyword(ctx, keywords[i].word, lval);
        if(!r)
            return keywords[i].token;

        if(r > 0)
            min = i+1;
        else
            max = i-1;
    }

    return 0;
}

static int parse_identifier(parser_ctx_t *ctx, const WCHAR **ret)
{
    const WCHAR *ptr = ctx->ptr++;
    WCHAR *str;
    int len;

    while(ctx->ptr < ctx->end && is_identifier_char(*ctx->ptr))
        ctx->ptr++;
    len = ctx->ptr-ptr;

    str = parser_alloc(ctx, (len+1)*sizeof(WCHAR));
    if(!str)
        return 0;

    memcpy(str, ptr, (len+1)*sizeof(WCHAR));
    str[len] = 0;
    *ret = str;
    return tIdentifier;
}

static int parse_string_literal(parser_ctx_t *ctx, const WCHAR **ret)
{
    const WCHAR *ptr = ++ctx->ptr;
    WCHAR *rptr;
    int len = 0;

    while(ctx->ptr < ctx->end) {
        if(*ctx->ptr == '\n' || *ctx->ptr == '\r') {
            FIXME("newline inside string literal\n");
            return 0;
        }

       if(*ctx->ptr == '"') {
            if(ctx->ptr[1] != '"')
                break;
            len--;
            ctx->ptr++;
        }
        ctx->ptr++;
    }

    if(ctx->ptr == ctx->end) {
        FIXME("unterminated string literal\n");
        return 0;
    }

    len += ctx->ptr-ptr;

    *ret = rptr = parser_alloc(ctx, (len+1)*sizeof(WCHAR));
    if(!rptr)
        return 0;

    while(ptr < ctx->ptr) {
        if(*ptr == '"')
            ptr++;
        *rptr++ = *ptr++;
    }

    *rptr = 0;
    ctx->ptr++;
    return tString;
}

static int parse_date_literal(parser_ctx_t *ctx, DATE *ret)
{
    const WCHAR *ptr = ++ctx->ptr;
    WCHAR *rptr;
    int len = 0;
    HRESULT res;

    while(ctx->ptr < ctx->end) {
        if(*ctx->ptr == '\n' || *ctx->ptr == '\r') {
            FIXME("newline inside date literal\n");
            return 0;
        }

       if(*ctx->ptr == '#')
            break;
       ctx->ptr++;
    }

    if(ctx->ptr == ctx->end) {
        FIXME("unterminated date literal\n");
        return 0;
    }

    len += ctx->ptr-ptr;

    rptr = malloc((len+1)*sizeof(WCHAR));
    if(!rptr)
        return 0;

    memcpy( rptr, ptr, len * sizeof(WCHAR));
    rptr[len] = 0;
    res = VarDateFromStr(rptr, ctx->lcid, 0, ret);
    free(rptr);
    if (FAILED(res)) {
        FIXME("Invalid date literal\n");
        return 0;
    }

    ctx->ptr++;
    return tDate;
}

static int parse_numeric_literal(parser_ctx_t *ctx, void **ret)
{
    BOOL use_int = TRUE;
    LONGLONG d = 0, hlp;
    int exp = 0;
    double r;

    if(*ctx->ptr == '0' && !('0' <= ctx->ptr[1] && ctx->ptr[1] <= '9') && ctx->ptr[1] != '.')
        return *ctx->ptr++;

    while(ctx->ptr < ctx->end && is_digit(*ctx->ptr)) {
        hlp = d*10 + *(ctx->ptr++) - '0';
        if(d>MAXLONGLONG/10 || hlp<0) {
            exp++;
            break;
        }
        else
            d = hlp;
    }
    while(ctx->ptr < ctx->end && is_digit(*ctx->ptr)) {
        exp++;
        ctx->ptr++;
    }

    if(*ctx->ptr == '.') {
        use_int = FALSE;
        ctx->ptr++;

        while(ctx->ptr < ctx->end && is_digit(*ctx->ptr)) {
            hlp = d*10 + *(ctx->ptr++) - '0';
            if(d>MAXLONGLONG/10 || hlp<0)
                break;

            d = hlp;
            exp--;
        }
        while(ctx->ptr < ctx->end && is_digit(*ctx->ptr))
            ctx->ptr++;
    }

    if(*ctx->ptr == 'e' || *ctx->ptr == 'E') {
        int e = 0, sign = 1;

        ctx->ptr++;
        if(*ctx->ptr == '-') {
            ctx->ptr++;
            sign = -1;
        }else if(*ctx->ptr == '+') {
            ctx->ptr++;
        }

        if(!is_digit(*ctx->ptr)) {
            FIXME("Invalid numeric literal\n");
            return 0;
        }

        use_int = FALSE;

        do {
            e = e*10 + *(ctx->ptr++) - '0';
            if(sign == -1 && -e+exp < -308) {
                /* The literal will be rounded to 0 anyway. */
                while(is_digit(*ctx->ptr))
                    ctx->ptr++;
                *(double*)ret = 0;
                return tDouble;
            }

            if(sign*e + exp > 308) {
                /* This would result in infinity */
                FIXME("Invalid numeric literal\n");
                return 0;
            }
        } while(is_digit(*ctx->ptr));

        exp += sign*e;
    }

    if(use_int && (LONG)d == d) {
        *(LONG*)ret = d;
        return tInt;
    }

    r = exp>=0 ? d*pow10[exp] : d/pow10[-exp];
    if(isinf(r)) {
        FIXME("Invalid numeric literal\n");
        return 0;
    }

    *(double*)ret = r;
    return tDouble;
}

static int hex_to_int(WCHAR c)
{
    if('0' <= c && c <= '9')
        return c-'0';
    if('a' <= c && c <= 'f')
        return c+10-'a';
    if('A' <= c && c <= 'F')
        return c+10-'A';
    return -1;
}

static int parse_hex_literal(parser_ctx_t *ctx, LONG *ret)
{
    const WCHAR *begin = ctx->ptr;
    unsigned l = 0, d;

    while((d = hex_to_int(*++ctx->ptr)) != -1)
        l = l*16 + d;

    if(begin + 9 /* max digits+1 */ < ctx->ptr) {
        FIXME("invalid literal\n");
        return 0;
    }

    if(*ctx->ptr == '&') {
        ctx->ptr++;
        *ret = l;
    }else {
        *ret = l == (UINT16)l ? (INT16)l : l;
    }
    return tInt;
}

static void skip_spaces(parser_ctx_t *ctx)
{
    while(*ctx->ptr == ' ' || *ctx->ptr == '\t')
        ctx->ptr++;
}

static int comment_line(parser_ctx_t *ctx)
{
    ctx->ptr = wcspbrk(ctx->ptr, L"\n\r");
    if(ctx->ptr)
        ctx->ptr++;
    else
        ctx->ptr = ctx->end;
    return tNL;
}

static int parse_next_token(void *lval, unsigned *loc, parser_ctx_t *ctx)
{
    WCHAR c;

    skip_spaces(ctx);
    *loc = ctx->ptr - ctx->code;
    if(ctx->ptr == ctx->end)
        return ctx->last_token == tNL ? 0 : tNL;

    c = *ctx->ptr;

    if('0' <= c && c <= '9')
        return parse_numeric_literal(ctx, lval);

    if(iswalpha(c)) {
        int ret = 0;
        if(ctx->last_token != '.' && ctx->last_token != tDOT)
            ret = check_keywords(ctx, lval);
        if(!ret)
            return parse_identifier(ctx, lval);
        if(ret != tREM)
            return ret;
        c = '\'';
    }

    switch(c) {
    case '\n':
    case '\r':
        ctx->ptr++;
        return tNL;
    case '\'':
        return comment_line(ctx);
    case ':':
    case ')':
    case ',':
    case '+':
    case '*':
    case '/':
    case '^':
    case '\\':
    case '_':
        return *ctx->ptr++;
    case '.':
        /*
         * We need to distinguish between '.' used as part of a member expression and
         * a beginning of a dot expression (a member expression accessing with statement
         * expression) and a floating point number like ".2" .
         */
        c = ctx->ptr > ctx->code ? ctx->ptr[-1] : '\n';
        if (is_identifier_char(c) || c == ')') {
            ctx->ptr++;
            return '.';
        }
        c = ctx->ptr[1];
        if('0' <= c && c <= '9')
            return parse_numeric_literal(ctx, lval);
        ctx->ptr++;
        return tDOT;
    case '-':
        if(ctx->is_html && ctx->ptr[1] == '-' && ctx->ptr[2] == '>')
            return comment_line(ctx);
        ctx->ptr++;
        return '-';
    case '(':
        /* NOTE:
         * We resolve empty brackets in lexer instead of parser to avoid complex conflicts
         * in call statement special case |f()| without 'call' keyword
         */
        ctx->ptr++;
        skip_spaces(ctx);
        if(*ctx->ptr == ')') {
            ctx->ptr++;
            return tEMPTYBRACKETS;
        }
        /*
         * Parser can't predict if bracket is part of argument expression or an argument
         * in call expression. We predict it here instead.
         */
        if(ctx->last_token == tIdentifier || ctx->last_token == ')')
            return '(';
        return tEXPRLBRACKET;
    case '"':
        return parse_string_literal(ctx, lval);
    case '#':
        return parse_date_literal(ctx, lval);
    case '&':
        if((*++ctx->ptr == 'h' || *ctx->ptr == 'H') && hex_to_int(ctx->ptr[1]) != -1)
            return parse_hex_literal(ctx, lval);
        return '&';
    case '=':
        switch(*++ctx->ptr) {
        case '<':
            ctx->ptr++;
            return tLTEQ;
        case '>':
            ctx->ptr++;
            return tGTEQ;
        }
        return '=';
    case '<':
        switch(*++ctx->ptr) {
        case '>':
            ctx->ptr++;
            return tNEQ;
        case '=':
            ctx->ptr++;
            return tLTEQ;
        case '!':
            if(ctx->is_html && ctx->ptr[1] == '-' && ctx->ptr[2] == '-')
                return comment_line(ctx);
        }
        return '<';
    case '>':
        switch(*++ctx->ptr) {
        case '=':
            ctx->ptr++;
            return tGTEQ;
        case '<':
            ctx->ptr++;
            return tNEQ;
        }
        return '>';
    default:
        FIXME("Unhandled char %c in %s\n", *ctx->ptr, debugstr_w(ctx->ptr));
    }

    return 0;
}

int parser_lex(void *lval, unsigned *loc, parser_ctx_t *ctx)
{
    int ret;

    if (ctx->last_token == tEXPRESSION)
    {
        ctx->last_token = tNL;
        return tEXPRESSION;
    }

    while(1) {
        ret = parse_next_token(lval, loc, ctx);
        if(ret == '_') {
            skip_spaces(ctx);
            if(*ctx->ptr != '\n' && *ctx->ptr != '\r') {
                FIXME("'_' not followed by newline\n");
                return 0;
            }
            if(*ctx->ptr == '\r')
                ctx->ptr++;
            if(*ctx->ptr == '\n')
                ctx->ptr++;
            continue;
        }
        if(ret != tNL || ctx->last_token != tNL)
            break;

        ctx->last_nl = ctx->ptr-ctx->code;
    }

    return (ctx->last_token = ret);
}
