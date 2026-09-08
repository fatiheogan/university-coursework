-- V2__email_verification_tokens.sql
-- Amaç: Register sonrası kullanıcıya bağlanan doğrulama tokenlarını tutmak.

CREATE TABLE IF NOT EXISTS email_verification_tokens (
  id         BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY,
  user_id    BIGINT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
  token      UUID   NOT NULL UNIQUE,
  expires_at TIMESTAMPTZ NOT NULL,
  used       BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE INDEX IF NOT EXISTS idx_evt_user ON email_verification_tokens(user_id);
