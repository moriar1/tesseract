# ErrorKit

Shared error codes for every Kit. One enum, human messages in one
place, machine names for page footers and logs.

```cpp
Tess::Error::Err err = Tess::Error::Err::ERR_UNKNOWN;
auto res = Tess::Net::FetchResponse(url, err);
if (!res) {
      ShowError(raw, err); // page footer prints Name(err)
}
```

## Codes

| Code | Meaning |
| --- | --- |
| `ERR_INVALID_URL` | Unparseable address |
| `ERR_NAME_NOT_RESOLVED` | DNS failed |
| `ERR_NO_NETWORK` | Refused, timeout, broken framing |
| `ERR_NOT_FOUND` | HTTP 404 (also missing `file://`) |
| `ERR_TIMEOUT` | Reserved: timed-out request |
| `ERR_UNSUPPORTED` | Scheme/transport not implemented (`https://`) |
| `ERR_UNKNOWN` | Fallback, never emit by choice |

## Rules

- Functions that can fail take `Tess::Error::Err &` and set it
  precisely — blanket `NO_NETWORK` is a bug (see `SocketConnect`).
- `Message(err)` is user text, `Name(err)` is the footer/log code.
  Add both arms when adding a code.
- Header-only (`Error.hpp`), no dependencies, no SDL.
