/** @file

  A brief file description

  @section license License

  Licensed to the Apache Software Foundation (ASF) under one
  or more contributor license agreements.  See the NOTICE file
  distributed with this work for additional information
  regarding copyright ownership.  The ASF licenses this file
  to you under the Apache License, Version 2.0 (the
  "License"); you may not use this file except in compliance
  with the License.  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
 */

#include "libts.h"
#include "ts/TestBox.h"

#if defined(darwin)
extern "C"
{
  struct hostent *gethostbyname_r(const char *name, struct hostent *result, char *buffer, int buflen, int *h_errnop);
  struct hostent *gethostbyaddr_r(const char *name, size_t size, int type,
                                  struct hostent *result, char *buffer, int buflen, int *h_errnop);
}
#endif

IpAddr const IpAddr::INVALID;

struct hostent *
ink_gethostbyname_r(char *hostname, ink_gethostbyname_r_data * data)
{
#ifdef RENTRENT_GETHOSTBYNAME
  struct hostent *r = gethostbyname(hostname);
  if (r)
    data->ent = *r;
  data->herrno = errno;

#else //RENTRENT_GETHOSTBYNAME
#if GETHOSTBYNAME_R_GLIBC2

  struct hostent *addrp = NULL;
  int res = gethostbyname_r(hostname, &data->ent, data->buf,
                            INK_GETHOSTBYNAME_R_DATA_SIZE, &addrp,
                            &data->herrno);
  struct hostent *r = NULL;
  if (!res && addrp)
    r = addrp;

#else
  struct hostent *r = gethostbyname_r(hostname, &data->ent, data->buf,
                                      INK_GETHOSTBYNAME_R_DATA_SIZE,
                                      &data->herrno);
#endif
#endif
  return r;
}

struct hostent *
ink_gethostbyaddr_r(char *ip, int len, int type, ink_gethostbyaddr_r_data * data)
{
#if GETHOSTBYNAME_R_GLIBC2
  struct hostent *r = NULL;
  struct hostent *addrp = NULL;
  int res = gethostbyaddr_r((char *) ip, len, type, &data->ent, data->buf,
                            INK_GETHOSTBYNAME_R_DATA_SIZE, &addrp,
                            &data->herrno);
  if (!res && addrp)
    r = addrp;
#else
#ifdef RENTRENT_GETHOSTBYADDR
  struct hostent *r = gethostbyaddr((const void *) ip, len, type);

#else
  struct hostent *r = gethostbyaddr_r((char *) ip, len, type, &data->ent,
                                      data->buf,
                                      INK_GETHOSTBYNAME_R_DATA_SIZE,
                                      &data->herrno);
#endif
#endif //LINUX
  return r;
}

uint32_t
ink_inet_addr(const char *s)
{
  uint32_t u[4];
  uint8_t *pc = (uint8_t *) s;
  int n = 0;
  uint32_t base = 10;

  while (n < 4) {

    u[n] = 0;
    base = 10;

    // handle hex, octal

    if (*pc == '0') {
      if (*++pc == 'x' || *pc == 'X')
        base = 16, pc++;
      else
        base = 8;
    }
    // handle hex, octal, decimal

    while (*pc) {
      if (ParseRules::is_digit(*pc)) {
        u[n] = u[n] * base + (*pc++ - '0');
        continue;
      }
      if (base == 16 && ParseRules::is_hex(*pc)) {
        u[n] = u[n] * 16 + ParseRules::ink_tolower(*pc++) - 'a' + 10;
        continue;
      }
      break;
    }

    n++;
    if (*pc == '.')
      pc++;
    else
      break;
  }

  if (*pc && !ParseRules::is_wslfcr(*pc))
    return htonl((uint32_t) - 1);

  switch (n) {
  case 1:
    return htonl(u[0]);
  case 2:
    if (u[0] > 0xff || u[1] > 0xffffff)
      return htonl((uint32_t) - 1);
    return htonl((u[0] << 24) | u[1]);
  case 3:
    if (u[0] > 0xff || u[1] > 0xff || u[2] > 0xffff)
      return htonl((uint32_t) - 1);
    return htonl((u[0] << 24) | (u[1] << 16) | u[2]);
  case 4:
    if (u[0] > 0xff || u[1] > 0xff || u[2] > 0xff || u[3] > 0xff)
      return htonl((uint32_t) - 1);
    return htonl((u[0] << 24) | (u[1] << 16) | (u[2] << 8) | u[3]);
  }
  return htonl((uint32_t) - 1);
}

const char *ats_ip_ntop(const struct sockaddr *addr, char *dst, size_t size)
{
  char const* zret = 0;

  switch (addr->sa_family) {
  case AF_INET:
    zret = inet_ntop(AF_INET, &ats_ip4_addr_cast(addr), dst, size);
    break;
  case AF_INET6:
    zret = inet_ntop(AF_INET6, &ats_ip6_addr_cast(addr), dst, size);
    break;
  default:
    zret = dst;
    snprintf(dst, size, "*Not IP address [%u]*", addr->sa_family);
    break;
  }
  return zret;
}

char const*
ats_ip_family_name(int family) {
  return AF_INET == family ? "IPv4"
    : AF_INET6 == family ? "IPv6"
    : "Unspec"
    ;
}

char const* ats_ip_nptop(
  sockaddr const* addr,
  char* dst, size_t size
) {
  char buff[INET6_ADDRSTRLEN];
  snprintf(dst, size, "%s:%u",
    ats_ip_ntop(addr, buff, sizeof(buff)),
    ats_ip_port_host_order(addr)
  );
  return dst;
}

int
ats_ip_parse(ts::ConstBuffer src, ts::ConstBuffer* addr, ts::ConstBuffer* port) {
  // In case the incoming arguments are null.
  ts::ConstBuffer localAddr, localPort;
  if (!addr) addr = &localAddr;
  if (!port) port = &localPort;
  addr->reset();
  port->reset();

  // Let's see if we can find out what's in the address string.
  if (src) {
    while (src && isspace(*src)) ++src;
    // Check for brackets.
    if ('[' == *src) {
      /* Ugly. In a number of places we must use bracket notation
         to support port numbers. Rather than mucking with that
         everywhere, we'll tweak it here. Experimentally we can't
         depend on getaddrinfo to handle it. Note that the text
         buffer size includes space for the nul, so a bracketed
         address is at most that size - 1 + 2 -> size+1.

         It just gets better. In order to bind link local addresses
         the scope_id must be set to the interface index. That's
         most easily done by appending a %intf (where "intf" is the
         name of the interface) to the address. Which makes
         the address potentially larger than the standard maximum.
         So we can't depend on that sizing.
      */
      ++src; // skip bracket.
      *addr = src.splitOn(']');
      if (*addr && ':' == *src) { // found the closing bracket and port colon
        ++src; // skip colon.
        *port = src;
      } // else it's a fail for unclosed brackets.
    } else {
      // See if there's exactly 1 colon
      ts::ConstBuffer tmp = src.after(':');
      if (tmp && ! tmp.find(':')) { // 1 colon and no others
        src.clip(tmp.data() - 1); // drop port from address.
        *port = tmp;
      } // else 0 or > 1 colon and no brackets means no port.
      *addr = src;
    }
    // clip port down to digits.
    if (*port) {
      char const* spot = port->data();
      while (isdigit(*spot)) ++spot;
      port->clip(spot);
    }
  }
  return *addr ? 0 : -1; // true if we found an address.
}

int
ats_ip_pton(char const* text, sockaddr* ip) {
  int zret = -1;
  ts::ConstBuffer addr, port;
  ts::ConstBuffer src(text, strlen(text)+1);

  ats_ip_invalidate(ip);
  if (0 == ats_ip_parse(src, &addr, &port)) {
    // Copy if not terminated.
    if (0 != addr[addr.size()-1]) {
      char* tmp = static_cast<char*>(alloca(addr.size()+1));
      memcpy(tmp, addr.data(), addr.size());
      tmp[addr.size()] = 0;
      addr.set(tmp, addr.size());
    }
    if (addr.find(':')) { // colon -> IPv6
      in6_addr addr6;
      if (inet_pton(AF_INET6, addr.data(), &addr6)) {
        zret = 0;
        ats_ip6_set(ip, addr6);
      }
    } else { // no colon -> must be IPv4
      in_addr addr4;
      if (inet_aton(addr.data(), &addr4)) {
        zret = 0;
        ats_ip4_set(ip, addr4.s_addr);
      }
    }
    // If we had a successful conversion, set the port.
    if (ats_is_ip(ip))
      ats_ip_port_cast(ip) = port ? htons(atoi(port.data())) : 0;
  }

  return zret;
}

uint32_t
ats_ip_hash(sockaddr const* addr) {
  union md5sum {
    unsigned char c[16];
    uint32_t i;
  } zret;
  zret.i = 0;

  if (ats_is_ip4(addr)) {
    zret.i = ats_ip4_addr_cast(addr);
  } else if (ats_is_ip6(addr)) {
    ink_code_md5(const_cast<uint8_t*>(ats_ip_addr8_cast(addr)), TS_IP6_SIZE, zret.c);
  }
  return zret.i;
}

int
ats_ip_to_hex(sockaddr const* src, char* dst, size_t len) {
  int zret = 0;
  ink_assert(len);
  char const* dst_limit = dst + len - 1; // reserve null space.
  if (ats_is_ip(src)) {
    uint8_t const* data = ats_ip_addr8_cast(src);
    for ( uint8_t const* src_limit = data + ats_ip_addr_size(src)
        ; data < src_limit && dst+1 < dst_limit
        ; ++data, zret += 2
    ) {
    uint8_t n1 = (*data >> 4) & 0xF; // high nybble.
    uint8_t n0 = *data & 0xF; // low nybble.

    *dst++ = n1 > 9 ? n1 + 'A' - 10 : n1 + '0';
    *dst++ = n0 > 9 ? n0 + 'A' - 10 : n0 + '0';
    }
  }
  *dst = 0; // terminate but don't include that in the length.
  return zret;
}

sockaddr* ats_ip_set(
  sockaddr* dst,
  IpAddr const& addr,
  uint16_t port
) {
  if (AF_INET == addr._family) ats_ip4_set(dst, addr._addr._ip4, port);
  else if (AF_INET6 == addr._family) ats_ip6_set(dst, addr._addr._ip6, port);
  else ats_ip_invalidate(dst);
  return dst;
}

int
IpAddr::load(char const* text) {
  IpEndpoint ip;
  int zret = ats_ip_pton(text, &ip);
  *this = ip;
  return zret;
}

char*
IpAddr::toString(char* dest, size_t len) const {
  IpEndpoint ip;
  ip.assign(*this);
  ats_ip_ntop(&ip, dest, len);
  return dest;
}

bool
IpAddr::isMulticast() const {
  return (AF_INET == _family && 0xe == _addr._byte[0]) ||
    (AF_INET6 == _family && IN6_IS_ADDR_MULTICAST(&_addr._ip6))
    ;
}

bool
operator == (IpAddr const& lhs, sockaddr const* rhs) {
  bool zret = false;
  if (lhs._family == rhs->sa_family) {
    if (AF_INET == lhs._family) {
      zret = lhs._addr._ip4 == ats_ip4_addr_cast(rhs);
    } else if (AF_INET6 == lhs._family) {
      zret = 0 == memcmp(&lhs._addr._ip6, &ats_ip6_addr_cast(rhs), sizeof(in6_addr));
    } else { // map all non-IP to the same thing.
      zret = true;
    }
  } // else different families, not equal.
  return zret;
}

int
ats_ip_getbestaddrinfo(char const* host,
  IpEndpoint* ip4,
  IpEndpoint* ip6
) {
  int zret = -1;
  int port = 0; // port value to assign if we find an address.
  addrinfo ai_hints;
  addrinfo* ai_result;
  ts::ConstBuffer addr_text, port_text;
  ts::ConstBuffer src(host, strlen(host)+1);

  if (ip4) ats_ip_invalidate(ip4);
  if (ip6) ats_ip_invalidate(ip6);

  if (0 == ats_ip_parse(src, &addr_text, &port_text)) {
    // Copy if not terminated.
    if (0 != addr_text[addr_text.size()-1]) {
      char* tmp = static_cast<char*>(alloca(addr_text.size()+1));
      memcpy(tmp, addr_text.data(), addr_text.size());
      tmp[addr_text.size()] = 0;
      addr_text.set(tmp, addr_text.size());
    }
    ink_zero(ai_hints);
    ai_hints.ai_family = AF_UNSPEC;
    ai_hints.ai_flags = AI_ADDRCONFIG;
    zret = getaddrinfo(addr_text.data(), 0, &ai_hints, &ai_result);
  
    if (0 == zret) {
      // Walk the returned addresses and pick the "best".
      enum {
        NA, // Not an (IP) Address.
        LO, // Loopback.
        LL, // Link Local.
        PR, // Private.
        MC, // Multicast.
        GL  // Global.
      } spot_type = NA, ip4_type = NA, ip6_type = NA;
      sockaddr const* ip4_src = 0;
      sockaddr const* ip6_src = 0;

      for ( addrinfo* ai_spot = ai_result
          ; ai_spot
          ; ai_spot = ai_spot->ai_next
      ) {
        sockaddr const* ai_ip = ai_spot->ai_addr;
        if (!ats_is_ip(ai_ip)) spot_type = NA;
        else if (ats_is_ip_loopback(ai_ip)) spot_type = LO;
        else if (ats_is_ip_linklocal(ai_ip)) spot_type = LL;
        else if (ats_is_ip_private(ai_ip)) spot_type = PR;
        else if (ats_is_ip_multicast(ai_ip)) spot_type = MC;
        else spot_type = GL;
        
        if (spot_type == NA) continue; // Next!

        if (ats_is_ip4(ai_ip)) {
          if (spot_type > ip4_type) {
            ip4_src = ai_ip;
            ip4_type = spot_type;
          }
        } else if (ats_is_ip6(ai_ip)) {
          if (spot_type > ip6_type) {
            ip6_src = ai_ip;
            ip6_type = spot_type;
          }
        }
      }
      if (ip4_type > NA) ats_ip_copy(ip4, ip4_src);
      if (ip6_type > NA) ats_ip_copy(ip6, ip6_src);
      freeaddrinfo(ai_result); // free *after* the copy.

    }
  }

  // We don't really care if the port is null terminated - the parser
  // would get all the digits so the next character is a non-digit (null or
  // not) and atoi will do the right thing in either case.
  if (port_text.size()) port = htons(atoi(port_text.data()));
  if (ats_is_ip(ip4)) ats_ip_port_cast(ip4) = port;
  if (ats_is_ip(ip6)) ats_ip_port_cast(ip6) = port;

  if (!ats_is_ip(ip4) && !ats_is_ip(ip6)) zret = -1;

  return zret;
}

int
ats_ip_check_characters(ts::ConstBuffer text) {
  bool found_colon = false;
  bool found_hex = false;
  for ( char const *p = text.data(), *limit = p + text.size()
      ; p < limit
      ; ++p
  )
    if (':' == *p) found_colon = true;
    else if ('.' == *p || isdigit(*p)) /* empty */;
    else if (isxdigit(*p)) found_hex = true;
    else return AF_UNSPEC;

  return found_hex && !found_colon ? AF_UNSPEC
    : found_colon ? AF_INET6
    : AF_INET
    ;
}

REGRESSION_TEST(Ink_Inet) (RegressionTest * t, int /* atype */, int * pstatus) {
  TestBox box(t, pstatus);
  IpEndpoint  ep;
  IpAddr      addr;

  box = REGRESSION_TEST_PASSED;

  box.check(ats_ip_pton("76.14.64.156", &ep.sa) == 0, "ats_ip_pton()");
  box.check(addr.load("76.14.64.156") == 0, "IpAddr::load()");
  box.check(addr.family() == ep.family(), "mismatched address family");

  switch (addr.family()) {
  case AF_INET:
    box.check(ep.sin.sin_addr.s_addr == addr._addr._ip4, "IPv4 address mismatch");
    break;
  case AF_INET6:
    box.check(memcmp(&ep.sin6.sin6_addr, &addr._addr._ip6, sizeof(in6_addr)) == 0, "IPv6 address mismatch");
    break;
  default:
    ;
  }
}
