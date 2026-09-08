package com.stackademy.stackademy.auth;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.nio.charset.StandardCharsets;
import java.security.Key;
import java.time.Instant;
import java.util.Date;
import java.util.Map;
import java.util.function.Function;

@Service
public class JwtService {
  private final Key key;
  private final long expirationMillis;

  public JwtService(
      @Value("${app.jwt.secret:BurasiCokGizliVeUzunBirSecretKeyOlmaliEnAz32Karakter}") String secret,
      @Value("${app.jwt.expiration-min:60}") long expirationMin
  ) {
    this.key = Keys.hmacShaKeyFor(secret.getBytes(StandardCharsets.UTF_8));
    this.expirationMillis = expirationMin * 60_000L;
  }

  public String generate(String subject, Map<String, Object> claims) {
    Instant now = Instant.now();
    return Jwts.builder()
        .subject(subject)
        .claims(claims)
        .issuedAt(Date.from(now))
        .expiration(Date.from(now.plusMillis(expirationMillis)))
        .signWith(key)
        .compact();
  }

  // --- Yeni Eklenen Metotlar (Doğrulama İçin) ---

  // Token'dan userId (subject) çeker
  public String extractSubject(String token) {
    return extractClaim(token, Claims::getSubject);
  }

  // Token geçerli mi?
  public boolean validate(String token, String userId) {
    final String subject = extractSubject(token);
    return (subject.equals(userId) && !isTokenExpired(token));
  }

  private boolean isTokenExpired(String token) {
    return extractExpiration(token).before(new Date());
  }

  private Date extractExpiration(String token) {
    return extractClaim(token, Claims::getExpiration);
  }

  private <T> T extractClaim(String token, Function<Claims, T> claimsResolver) {
    final Claims claims = extractAllClaims(token);
    return claimsResolver.apply(claims);
  }

  private Claims extractAllClaims(String token) {
    return Jwts.parser()
        .verifyWith((javax.crypto.SecretKey) key)
        .build()
        .parseSignedClaims(token)
        .getPayload();
  }
}