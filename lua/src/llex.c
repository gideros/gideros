/*
** $Id: llex.c,v 2.20.1.1 2007/12/27 13:02:25 roberto Exp $
** Lexical Analyzer
** See Copyright Notice in lua.h
*/


#include <ctype.h>
#include <locale.h>
#include <string.h>

#define llex_c
#define LUA_CORE

#include "lua.h"

#include "ldo.h"
#include "llex.h"
#include "lobject.h"
#include "lparser.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lzio.h"
#include "lauxlib.h"


#define next(ls) (ls->current = ls->mpos < ls->mlen ? char2int(ls->mstr[ls->mpos++]) : zgetc(ls->z))

#define currIsNewline(ls)	(ls->current == '\n' || ls->current == '\r')

#define MACRO "MACRO"

/* ORDER RESERVED */
const char *const luaX_tokens [] = {
  "and", "break", "do", "else", "elseif",
  "end", "false", "for", "function", "if",
  "in", "local", "nil", "not", "or", "repeat",
  "return", "then", "true", "until", "while",
  "..", "...", "==", ">=", "<=", "~=",
  "<<", ">>", "//",
  "<number>", "<name>", "<string>", "<eof>",
  NULL
};


#define save_and_next(ls) (save(ls, ls->current), next(ls))


static void save (LexState *ls, int c) {
  Mbuffer *b = ls->buff;
  if (b->n + 1 > b->buffsize) {
    size_t newsize;
    if (b->buffsize >= MAX_SIZET/2)
      luaX_lexerror(ls, "lexical element too long", 0);
    newsize = b->buffsize * 2;
    luaZ_resizebuffer(ls->L, b, newsize);
  }
  b->buffer[b->n++] = cast(char, c);
}


void luaX_init (lua_State *L) {
  int i;
  for (i=0; i<NUM_RESERVED; i++) {
    TString *ts = luaS_new(L, luaX_tokens[i]);
    luaS_fix(ts);  /* reserved words are never collected */
    lua_assert(strlen(luaX_tokens[i])+1 <= TOKEN_LEN);
    ts->tsv.reserved = cast_byte(i+1);  /* reserved word */
  }
}


#define MAXSRC          256


const char *luaX_token2str (LexState *ls, int token) {
  if (token < FIRST_RESERVED) {
    lua_assert(token == cast(unsigned char, token));
    return (iscntrl(token)) ? luaO_pushfstring(ls->L, "char(%d)", token) :
                              luaO_pushfstring(ls->L, "%c", token);
  }
  else
    return luaX_tokens[token-FIRST_RESERVED];
}


static const char *txtToken (LexState *ls, int token) {
  switch (token) {
  case TK_NAME:
  case TK_STRING:
  case TK_NUMBER:
    save(ls, '\0');
    return luaZ_buffer(ls->buff);
  default:
    return luaX_token2str(ls, token);
  }
}


void luaX_lexerror (LexState *ls, const char *msg, int token) {
  char buff[MAXSRC];
  luaO_chunkid(buff, getstr(ls->source), MAXSRC);
  msg = luaO_pushfstring(ls->L, "%s:%d: %s", buff, ls->linenumber, msg);
  if (token)
    luaO_pushfstring(ls->L, "%s near " LUA_QS, msg, txtToken(ls, token));
  luaD_throw(ls->L, LUA_ERRSYNTAX);
}


void luaX_syntaxerror (LexState *ls, const char *msg) {
  luaX_lexerror(ls, msg, ls->t.token);
}


TString *luaX_newstring (LexState *ls, const char *str, size_t l) {
  lua_State *L = ls->L;
  TString *ts = luaS_newlstr(L, str, l);
  TValue *o = luaH_setstr(L, ls->fs->h, ts);  /* entry for `str' */
  if (ttisnil(o))
    setbvalue(o, 1);  /* make sure `str' will not be collected */
  return ts;
}


static void inclinenumber (LexState *ls) {
  int old = ls->current;
  lua_assert(currIsNewline(ls));
  next(ls);  /* skip `\n' or `\r' */
  if (currIsNewline(ls) && ls->current != old)
    next(ls);  /* skip `\n\r' or `\r\n' */
  if (ls->mpos < ls->mlen) return;
  if (++ls->linenumber >= MAX_INT)
    luaX_syntaxerror(ls, "chunk has too many lines");
}


void luaX_setinput (lua_State *L, LexState *ls, ZIO *z, TString *source) {
  ls->decpoint = '.';
  ls->L = L;
  ls->lookahead.token = TK_EOS;  /* no look-ahead token */
  ls->z = z;
  ls->fs = NULL;
  ls->linenumber = 1;
  ls->lastline = 1;
  ls->source = source;
  luaZ_resizebuffer(ls->L, ls->buff, LUA_MINBUFFER);  /* initialize buffer */
  next(ls);  /* read first char */
}



/*
** =======================================================
** LEXICAL ANALYZER
** =======================================================
*/



static int check_next (LexState *ls, const char *set) {
  if (!strchr(set, ls->current))
    return 0;
  save_and_next(ls);
  return 1;
}


static void buffreplace (LexState *ls, char from, char to) {
  size_t n = luaZ_bufflen(ls->buff);
  char *p = luaZ_buffer(ls->buff);
  while (n--)
    if (p[n] == from) p[n] = to;
}


static void trydecpoint (LexState *ls, SemInfo *seminfo) {
  /* format error: try to update decimal point separator */
  char old = ls->decpoint;
#ifdef __ANDROID__
  ls->decpoint = '.';
#else
  struct lconv *cv = localeconv();
  ls->decpoint = (cv ? cv->decimal_point[0] : '.');
#endif
  buffreplace(ls, old, ls->decpoint);  /* try updated decimal separator */
  if (!luaO_str2d(luaZ_buffer(ls->buff), &seminfo->r)) {
    /* format error with correct decimal point: no more options */
    buffreplace(ls, ls->decpoint, '.');  /* undo change (for error message) */
    luaX_lexerror(ls, "malformed number", TK_NUMBER);
  }
}


/* LUA_NUMBER */
static void read_numeral (LexState *ls, SemInfo *seminfo) {
  lua_assert(isdigit(ls->current));
  do {
    save_and_next(ls);
  } while (isdigit(ls->current) || ls->current == '.');
  if (check_next(ls, "Ee"))  /* `E'? */
    check_next(ls, "+-");  /* optional exponent sign */
  while (isalnum(ls->current) || ls->current == '_')
    save_and_next(ls);
  save(ls, '\0');
  buffreplace(ls, '.', ls->decpoint);  /* follow locale for decimal point */
  if (!luaO_str2d(luaZ_buffer(ls->buff), &seminfo->r))  /* format error? */
    trydecpoint(ls, seminfo); /* try to update decimal point separator */
}


static int skip_sep (LexState *ls) {
  int count = 0;
  int s = ls->current;
  lua_assert(s == '[' || s == ']');
  save_and_next(ls);
  while (ls->current == '=') {
    save_and_next(ls);
    count++;
  }
  return (ls->current == s) ? count : (-count) - 1;
}


static void read_long_string (LexState *ls, SemInfo *seminfo, int sep) {
  int cont = 0;
  (void)(cont);  /* avoid warnings when `cont' is not used */
  save_and_next(ls);  /* skip 2nd `[' */
  if (currIsNewline(ls))  /* string starts with a newline? */
    inclinenumber(ls);  /* skip it */
  for (;;) {
    switch (ls->current) {
    case EOZ:
      luaX_lexerror(ls, (seminfo) ? "unfinished long string" :
                                    "unfinished long comment", TK_EOS);
      break;  /* to avoid warnings */
#if defined(LUA_COMPAT_LSTR)
    case '[': {
      if (skip_sep(ls) == sep) {
        save_and_next(ls);  /* skip 2nd `[' */
        cont++;
#if LUA_COMPAT_LSTR == 1
        if (sep == 0)
          luaX_lexerror(ls, "nesting of [[...]] is deprecated", '[');
#endif
      }
      break;
    }
#endif
    case ']': {
      if (skip_sep(ls) == sep) {
        save_and_next(ls);  /* skip 2nd `]' */
#if defined(LUA_COMPAT_LSTR) && LUA_COMPAT_LSTR == 2
        cont--;
        if (sep == 0 && cont >= 0) break;
#endif
        goto endloop;
      }
      break;
    }
    case '\n':
    case '\r': {
      save(ls, '\n');
      inclinenumber(ls);
      if (!seminfo) luaZ_resetbuffer(ls->buff);  /* avoid wasting space */
      break;
    }
    default: {
      if (seminfo) save_and_next(ls);
      else next(ls);
    }
    }
  } endloop:
    if (seminfo)
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + (2 + sep),
    luaZ_bufflen(ls->buff) - 2*(2 + sep));
}


static void read_string (LexState *ls, int del, SemInfo *seminfo) {
  save_and_next(ls);
  while (ls->current != del) {
    switch (ls->current) {
    case EOZ:
      luaX_lexerror(ls, "unfinished string", TK_EOS);
      continue;  /* to avoid warnings */
    case '\n':
    case '\r':
      luaX_lexerror(ls, "unfinished string", TK_STRING);
      continue;  /* to avoid warnings */
    case '\\': {
      int c;
      next(ls);  /* do not save the `\' */
      switch (ls->current) {
      case 'a': c = '\a'; break;
      case 'b': c = '\b'; break;
      case 'f': c = '\f'; break;
      case 'n': c = '\n'; break;
      case 'r': c = '\r'; break;
      case 't': c = '\t'; break;
      case 'v': c = '\v'; break;
      case '\n':  /* go through */
      case '\r': save(ls, '\n'); inclinenumber(ls); continue;
      case EOZ: continue;  /* will raise an error next loop */
      default: {
        if (!isdigit(ls->current))
          save_and_next(ls);  /* handles \\, \", \', and \? */
        else {  /* \xxx */
          int i = 0;
          c = 0;
          do {
            c = 10*c + (ls->current-'0');
            next(ls);
          } while (++i<3 && isdigit(ls->current));
          if (c > UCHAR_MAX)
            luaX_lexerror(ls, "escape sequence too large", TK_STRING);
          save(ls, c);
        }
        continue;
      }
      }
      save(ls, c);
      next(ls);
      continue;
    }
    default:
      save_and_next(ls);
    }
  }
  save_and_next(ls);  /* skip delimiter */
  seminfo->ts = luaX_newstring(ls, luaZ_buffer(ls->buff) + 1,
                               luaZ_bufflen(ls->buff) - 2);
}


static int llex (LexState *ls, SemInfo *seminfo) {
  luaZ_resetbuffer(ls->buff);
  for (;;) {
    switch (ls->current) {
    case '\n':
    case '\r': {
      inclinenumber(ls);
      continue;
    }
    case '-': {
      next(ls);
      if (ls->current != '-') return '-';
      /* else is a comment */
      next(ls);
      if (ls->current == '[') {
        int sep = skip_sep(ls);
        luaZ_resetbuffer(ls->buff);  /* `skip_sep' may dirty the buffer */
        if (sep >= 0) {
          read_long_string(ls, NULL, sep);  /* long comment */
          luaZ_resetbuffer(ls->buff);
          continue;
        }
      }
      /* else short comment */
      while (!currIsNewline(ls) && ls->current != EOZ)
        next(ls);
      continue;
    }
    case '[': {
      int sep = skip_sep(ls);
      if (sep >= 0) {
        read_long_string(ls, seminfo, sep);
        return TK_STRING;
      }
      else if (sep == -1) return '[';
      else luaX_lexerror(ls, "invalid long string delimiter", TK_STRING);
    }
    case '=': {
      next(ls);
      if (ls->current != '=') return '=';
      else { next(ls); return TK_EQ; }
    }
    case '<': {
      next(ls);
      if (ls->current == '=') { next(ls); return TK_LE; }
      else if (ls->current == '<') { next(ls); return TK_LSHFT; }
      else  return '<';
    }
    case '>': {
      next(ls);
      if (ls->current == '=') { next(ls); return TK_GE; }
      else if (ls->current == '>') { next(ls); return TK_RSHFT; }
      else return '>';
    }
    case '/': {
      next(ls);
      if (ls->current != '/') return '/';
      else { next(ls); return TK_INTDIV; }
    }
    case '~': {
      next(ls);
      if (ls->current != '=') return '~';
      else { next(ls); return TK_NE; }
    }
    case '!': {
      luaX_lexerror(ls, "assembler support is not implemented yet", 0);
    }
    case '"':
    case '\'': {
      read_string(ls, ls->current, seminfo);
      return TK_STRING;
    }
    case '.': {
      save_and_next(ls);
      if (check_next(ls, ".")) {
        if (check_next(ls, "."))
          return TK_DOTS;   /* ... */
        else return TK_CONCAT;   /* .. */
      }
      else if (!isdigit(ls->current)) return '.';
      else {
        read_numeral(ls, seminfo);
        return TK_NUMBER;
      }
    }
    case '@': {
      luaX_lexerror(ls, "invalid macro identifier", 0);
    }
    case EOZ: {
      return TK_EOS;
    }
    default: {
      if (isspace(ls->current)) {
        lua_assert(!currIsNewline(ls));
        next(ls);
        continue;
      }
      else if (isdigit(ls->current)) {
        read_numeral(ls, seminfo);
        return TK_NUMBER;
      }
      else if (isalpha(ls->current) || ls->current == '_') {
        TString *ts;
        do {
          save_and_next(ls);
        } while (isalnum(ls->current) || ls->current == '_');
        ts = luaX_newstring(ls, luaZ_buffer(ls->buff),
                            luaZ_bufflen(ls->buff));
        while (isspace(ls->current)) {
          if (ls->current == '\n') ++ls->linenumber;
          next(ls);
        }
        if (ls->current == '@') {
          if (ts->tsv.reserved > 0)
            luaX_lexerror(ls, "invalid macro identifier",
                          ts->tsv.reserved - 1 + FIRST_RESERVED);
          lua_State *L = ls->L;
          lua_getglobal(L, MACRO);

          if(!lua_istable(L,-1))
          {
            lua_pop(L, 1);
            lua_newtable(L);
            lua_setglobal(L, MACRO);
            lua_getglobal(L, MACRO);
          }
          next(ls);
          save(ls, '\0');
          char *key = (char *) malloc(strlen(ls->buff->buffer) + 1);
          if (key) strcpy(key, ls->buff->buffer);
          else luaX_lexerror(ls, "cannot allocate enough memory",0);
          lua_pushstring(L, key);
          lua_gettable(L, -2);
          if (!lua_isnil(L, -1))
          {
            if (ls->current != '@')
              luaX_lexerror(ls, "definition of existent macro", 0);
          } else if (ls->current == '@')
            luaX_lexerror(ls, "redefinition of nonexistent macro", 0);
          if (ls->current == '@') next(ls);
          lua_pop(L, 1);
          lua_pushstring(L, key);
          luaZ_resetbuffer(ls->buff);
          while (isspace(ls->current))
          {
            if (ls->current == '\n') ++ls->linenumber;
            next(ls);
          }
          char q = ls->current;
          switch (q)
          {
          case '(':
          {
            next(ls);
            q = ls->current;
            next(ls);
            for(;;) {
              if (ls->current == q) {
                next(ls);
                if (ls->current == ')') {
                  next(ls); break;
                } else {
                  save(ls, q);
                  save_and_next(ls);
                }
              } else if (ls->current == EOZ)
                luaX_lexerror(ls, "unfinished macro function definition", 0);
              else if (ls->current == '\n') {
                ++ls->linenumber;
                save_and_next(ls);
              }
              else save_and_next(ls);
            }
            q = '(';
            break;
          }
          case '\'':
          case '"':
          {
            save_and_next(ls);
            while (ls->current != q)
            {
              if (ls->current == EOZ)
                luaX_lexerror(ls, "unfinished string at macro definition", 0);
              else if (ls->current == '\n')
                luaX_lexerror(ls, "unfinished string at macro definition", TK_STRING);
              else if (ls->current == '\\') {
                save_and_next(ls);
                if (ls->current == EOZ)
                  luaX_lexerror(ls, "unfinished string at macro definition", TK_STRING);
              }
              save_and_next(ls);
            }
            save_and_next(ls);
            break;
          }
          case '[':
          {
            save_and_next(ls);
            if (ls->current != '[' && ls->current != '=')
              luaX_lexerror(ls, "invalid long string at macro definition", 0);
            int eqlen = 0;
            int eqnum = 0;
            while (ls->current == '=') {eqlen++; save_and_next(ls);};
            if (eqlen > 0 && ls->current != '[')
              luaX_lexerror(ls, "invalid long string at macro definition", 0);
            save_and_next(ls);
            for(;;)
            {
              switch (ls->current)
              {
              case EOZ:
                luaX_lexerror(ls, "unfinished long string at macro definition", 0);
                break;
              case '\n':
                ls->linenumber++;
                break;
              case '=':
                ++eqnum;
                break;
              case ']':
                if (eqnum == -1) eqnum = 0;
              }
              if (ls->current != '=' && ls->current != ']') eqnum = -1;
              save_and_next(ls);
              if (eqnum == eqlen && ls->current == ']')
              {
                save_and_next(ls);
                break;
              }
            }
            break;
          }
          case '-': case '0': case '1': case '2': case '3': case '4':
          case '.': case '5': case '6': case '7': case '8': case '9':
          {
            if (q == '-') {
              save_and_next(ls);
              if (!isdigit(ls->current) && ls->current != '.')
                luaX_lexerror(ls, "invalid number at macro definition", 0);
            }
            while (isalnum(ls->current) || ls->current == '.') save_and_next(ls);
            break;
          }
          case '`': case '~': case '!': case '#': case '$': case '%': case '^':
          case '&': case '*': case '/': case '+': case '=': case '|': case '\\':
          {
            next(ls);
            while (ls->current != q)
            {
              if (ls->current == EOZ)
                luaX_lexerror(ls, "unfinished macro definition", 0);
              if (ls->current == '\n') ++ls->linenumber;
              save_and_next(ls);
            }
            next(ls);
            break;
          }
          default:
          {
            if (isalpha(ls->current) || ls->current == '_') {
              do {
                save_and_next(ls);
              } while (isalnum(ls->current) || ls->current == '_');
            } else luaX_lexerror(ls, "invalid macro marker", 0);
          }
          }
          save(ls, ' ');
          save(ls, '\0');
          char *val = (char *) malloc(strlen(ls->buff->buffer) + 1);
          if(val) strcpy(val, ls->buff->buffer);
          lua_pushstring(L, val);
          luaZ_resetbuffer(ls->buff);
          if (q == '(') {
            if (luaL_loadstring(L, val))
              luaX_lexerror(ls, lua_tostring(L, -1), 0);
            else
              lua_remove(L, -2);
          }
          lua_rawset(L, -3);
          lua_setglobal(L, MACRO);
          free(key);
          free(val);
          continue;
        }
        if (ts->tsv.reserved > 0)  /* reserved word? */
          return ts->tsv.reserved - 1 + FIRST_RESERVED;
        lua_State *L = ls->L;
        lua_getglobal(L, MACRO);
        if (lua_istable(L, -1)) {
          save(ls, '\0');
          char *k = ls->buff->buffer;
          lua_getfield(L, -1, k);

          if (lua_isfunction(L, -1)) {
            lua_newtable(L);
            lua_pushstring(L,getstr(ls->source));
            lua_rawseti(L,-2,0);
            if (ls->current != '(')
              luaX_lexerror(ls, "invalid macro function call", 0);
            int parens = 1;
            next(ls);
            int t, i;
            for (i = 1; ;i++) {
              t = llex(ls, seminfo);
              if (t == ')' && --parens == 0) break;
              else if (t == '(') ++parens;
              switch (t)
              {
              case TK_EOS: case EOZ:
                luaX_lexerror(ls, "unfinished macro function call", 0);
              case TK_STRING:
              {
                const char *str = getstr(seminfo->ts);
                TString *ts = seminfo->ts;
                size_t len = ts->tsv.len;
                char *res = (char *) malloc(4 * len + 2);
                res[0] = '"';
                size_t j = 0;
                size_t i;
                for (i = 0; i < len; i++) {
                  switch (str[i])
                  {
                  case '\a': res[++j] = '\\'; res[++j] = 'a'; break;
                  case '\b': res[++j] = '\\'; res[++j] = 'b'; break;
                  case '\f': res[++j] = '\\'; res[++j] = 'f'; break;
                  case '\t': res[++j] = '\\'; res[++j] = 't'; break;
                  case '\v': res[++j] = '\\'; res[++j] = 'v'; break;
                  case '\n': res[++j] = '\\'; res[++j] = 'n'; break;
                  case '\r': res[++j] = '\\'; res[++j] = 'r'; break;
                  case  '"': res[++j] = '\\'; res[++j] = '"'; break;
                  case '\0':
                    res[++j] = '\\';
                    res[++j] = '0';
                    res[++j] = '0';
                    res[++j] = '0';
                    break;
                  default:
                    res[++j] = str[i];
                    break;
                  }
                }
                res[++j] = '"';
                res[++j] = '\0';
                lua_pushstring(L, res);
                free(res);
                break;
              }
              case TK_NAME:
                lua_pushstring(L,getstr(seminfo->ts));
                break;
              case TK_NUMBER:
              {
                const char *str = ls->buff->buffer;
                if (str[0] == '0') lua_pushstring(L,str);
                else {
                  char *res = (char *) malloc(strlen(str) + 1);
                  res[0] = '\0';
                  strcat(res, "0");
                  strcat(res, str);
                  lua_pushstring(L,res);
                }
                break;
              }
              default:
                if (t<FIRST_RESERVED)
                {
                  char s[2]= {(char)t,0};
                  lua_pushstring(L,s);
                }
                else
                  lua_pushstring(L,luaX_tokens[t-FIRST_RESERVED]);
                break;
              }
              lua_rawseti(L,-2,i);
            }
            lua_call(L,1,1);
            if (!lua_isstring(L, -1))
              luaX_lexerror(ls, "macro function call should return string", 0);
            lua_pushstring(L, " ");
            lua_concat(L, 2);
          }

          if (lua_isstring(L, -1)) {
            luaZ_resetbuffer(ls->buff);
            const char *s = lua_tostring(L, -1);
            if (ls->mpos < ls->mlen) {
              int len = strlen(s) + ls->mlen - ls->mpos + 1;
              char *res = (char*) malloc(len);
              strcpy(res, s);
              strcat(res, &ls->mstr[ls->mpos-1]);
              strcat(res, " ");
              free(ls->mstr);
              ls->mstr = res;
              ls->z->n--; ls->z->p++;
            }
            else {
              if (ls->mlen > 0) free(ls->mstr);
              char *res = (char*) malloc(strlen(s)+1);
              strcpy(res, s);
              ls->mstr = res;
            }
            ls->mpos = 1;
            ls->mlen = strlen(ls->mstr);
            if (ls->mlen > 0) {
              ls->current = ls->mstr[0];
              ls->z->n++; ls->z->p--;
            }
            lua_pop(L, 1);
            lua_pop(L, 1);
            if (ls->mswt++ > 1e6) luaX_lexerror(ls, "possible mutual macro recursion", 0);
            continue;
          }
          lua_pop(L, 1);
        }
        lua_pop(L, 1);

        seminfo->ts = ts;
        return TK_NAME;
      }
      else {
        int c = ls->current;
        next(ls);
        return c;  /* single-char tokens (+ - / ...) */
      }
    }
    }
  }
}


void luaX_next (LexState *ls) {
  ls->lastline = ls->linenumber;
  if (ls->lookahead.token != TK_EOS) {  /* is there a look-ahead token? */
    ls->t = ls->lookahead;  /* use this one */
    ls->lookahead.token = TK_EOS;  /* and discharge it */
  }
  else
    ls->t.token = llex(ls, &ls->t.seminfo);  /* read next token */
}


void luaX_lookahead (LexState *ls) {
  lua_assert(ls->lookahead.token == TK_EOS);
  ls->lookahead.token = llex(ls, &ls->lookahead.seminfo);
}

