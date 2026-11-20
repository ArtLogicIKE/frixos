/* ============================================================
   auth.js — access password. Used by login.html and the settings page.
   The device stores only hash = PBKDF2-HMAC-SHA256(password, salt, iter), where
   salt is "frixos:" + its MAC. To log in, the browser fetches a one-time nonce
   and sends HMAC-SHA256(hash, nonce), so the password never leaves the browser.
   crypto.subtle only exists on https or localhost, not on the device's plain-http
   page, so SHA-256, HMAC and PBKDF2 are implemented here.
   ============================================================ */
(function (g) {
  const K = new Uint32Array([
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2]);
  const rotr = (x, n) => (x >>> n) | (x << (32 - n));

  function sha256(msg) {
    const len = msg.length, total = ((len + 9 + 63) >> 6) << 6;
    const buf = new Uint8Array(total);
    buf.set(msg); buf[len] = 0x80;
    const dv = new DataView(buf.buffer);
    dv.setUint32(total - 8, Math.floor(len / 0x20000000));
    dv.setUint32(total - 4, (len * 8) >>> 0);
    const H = new Uint32Array([0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19]);
    const W = new Uint32Array(64);
    for (let o = 0; o < total; o += 64) {
      for (let i = 0; i < 16; i++) W[i] = dv.getUint32(o + i * 4);
      for (let i = 16; i < 64; i++) {
        const x = W[i - 15], y = W[i - 2];
        W[i] = W[i - 16] + (rotr(x, 7) ^ rotr(x, 18) ^ (x >>> 3)) + W[i - 7] + (rotr(y, 17) ^ rotr(y, 19) ^ (y >>> 10));
      }
      let a = H[0], b = H[1], c = H[2], d = H[3], e = H[4], f = H[5], h7 = H[6], h = H[7];
      for (let i = 0; i < 64; i++) {
        const t1 = (h + (rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25)) + ((e & f) ^ (~e & h7)) + K[i] + W[i]) | 0;
        const t2 = ((rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22)) + ((a & b) ^ (a & c) ^ (b & c))) | 0;
        h = h7; h7 = f; f = e; e = (d + t1) | 0; d = c; c = b; b = a; a = (t1 + t2) | 0;
      }
      H[0] += a; H[1] += b; H[2] += c; H[3] += d; H[4] += e; H[5] += f; H[6] += h7; H[7] += h;
    }
    const out = new Uint8Array(32);
    const odv = new DataView(out.buffer);
    for (let i = 0; i < 8; i++) odv.setUint32(i * 4, H[i]);
    return out;
  }

  function concat(a, b) { const r = new Uint8Array(a.length + b.length); r.set(a); r.set(b, a.length); return r; }

  function hmac(key, msg) {
    if (key.length > 64) key = sha256(key);
    const ipad = new Uint8Array(64).fill(0x36), opad = new Uint8Array(64).fill(0x5c);
    for (let i = 0; i < key.length; i++) { ipad[i] ^= key[i]; opad[i] ^= key[i]; }
    return sha256(concat(opad, sha256(concat(ipad, msg))));
  }

  // One 32-byte block is all we need (dkLen = hash length).
  function pbkdf2(pw, salt, iter) {
    let u = hmac(pw, concat(salt, new Uint8Array([0, 0, 0, 1])));
    const t = u.slice();
    for (let i = 1; i < iter; i++) {
      u = hmac(pw, u);
      for (let j = 0; j < 32; j++) t[j] ^= u[j];
    }
    return t;
  }

  const utf8 = s => new TextEncoder().encode(s);
  const hex = b => Array.from(b, x => x.toString(16).padStart(2, '0')).join('');
  const unhex = h => new Uint8Array((h.match(/../g) || []).map(x => parseInt(x, 16)));

  const hashPassword = (pw, c) => pbkdf2(utf8(pw), utf8(c.salt), c.iter);
  const proof = (hash, nonce) => hex(hmac(hash, unhex(nonce)));

  async function post(url, obj) {
    const r = await fetch(url, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify(obj || {}) });
    let data = {}; try { data = await r.json(); } catch (e) { /* empty body */ }
    return { ok: r.ok, status: r.status, data };
  }

  async function challenge() {
    const r = await fetch('/api/auth/challenge?_=' + Date.now(), { cache: 'no-store' });
    return r.json();
  }

  /* Returns {ok, status, data}; data.locked is the lockout in seconds. */
  async function login(pw) {
    const c = await challenge();
    if (c.locked) return { ok: false, status: 429, data: { locked: c.locked } };
    return post('/api/auth/login', { nonce: c.nonce, proof: proof(hashPassword(pw, c), c.nonce) });
  }

  /* Sets (newPw) or removes (newPw === '') the password. currentPw is required when one is set. */
  async function setPassword(newPw, currentPw) {
    const c = await challenge();
    const body = { hash: newPw ? hex(hashPassword(newPw, c)) : '' };
    if (c.auth) { body.nonce = c.nonce; body.proof = proof(hashPassword(currentPw || '', c), c.nonce); }
    return post('/api/auth/password', body);
  }

  const logout = () => post('/api/auth/logout');

  g.frixosAuth = { sha256, hmac, pbkdf2, hex, utf8, challenge, login, setPassword, logout };
})(typeof window !== 'undefined' ? window : globalThis);
