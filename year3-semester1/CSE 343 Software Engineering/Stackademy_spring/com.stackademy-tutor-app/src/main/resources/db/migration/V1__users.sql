-- V1__users.sql
-- Amaç: Uygulamanın temel kullanıcı tablosunu oluşturmak.

CREATE TABLE IF NOT EXISTS users (
  user_id       BIGINT GENERATED ALWAYS AS IDENTITY PRIMARY KEY, -- otomatik artan PK
  first_name    VARCHAR(100),
  last_name     VARCHAR(100),

  -- email ve nickname'i normal VARCHAR olarak tanımlıyoruz (citext gerekmesin diye)
  nickname      VARCHAR(120),
  email         VARCHAR(255),

  phone         VARCHAR(20),
  password_hash VARCHAR(255),  -- şifre hash'i (gerçek şifre değil)
  user_type     TEXT NOT NULL CHECK (user_type IN ('free','premium','exclusive','teacher')), -- üyelik tipi
  created_at    TIMESTAMPTZ NOT NULL DEFAULT NOW(),  -- kayıt tarihi (timezone'lı)
  is_active     BOOLEAN NOT NULL DEFAULT TRUE         -- aktif/pasif
);

-- Benzersiz index: küçük/büyük harf duyarsız (LOWER ile)
CREATE UNIQUE INDEX IF NOT EXISTS ux_users_email_lower
  ON users (LOWER(email));
CREATE UNIQUE INDEX IF NOT EXISTS ux_users_nickname_lower
  ON users (LOWER(nickname));

-- Sorgu hızlandırma için ek index’ler (opsiyonel ama faydalı)
CREATE INDEX IF NOT EXISTS idx_users_created_at ON users (created_at);
CREATE INDEX IF NOT EXISTS idx_users_user_type  ON users (user_type);
