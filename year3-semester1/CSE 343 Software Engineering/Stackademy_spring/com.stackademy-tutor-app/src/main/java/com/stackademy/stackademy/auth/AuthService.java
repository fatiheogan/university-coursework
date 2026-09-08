package com.stackademy.stackademy.auth;

import org.springframework.security.authentication.AuthenticationManager;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import com.stackademy.stackademy.auth.dto.AuthResponse;
import com.stackademy.stackademy.auth.dto.LoginRequest;
import com.stackademy.stackademy.auth.dto.RegisterRequest;
import com.stackademy.stackademy.mail.EmailService;
import com.stackademy.stackademy.user.User;
import com.stackademy.stackademy.user.UserRepository;

import java.time.OffsetDateTime;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

@Service
public class AuthService {

    private final EmailService emailService;
    private final UserRepository userRepo;
    private final EmailVerificationTokenRepository tokenRepo;
    private final PasswordEncoder encoder;
    private final JwtService jwt;
    // AuthManager login işlemi için gereklidir
    private final AuthenticationManager authenticationManager;

    public AuthService(EmailService emailService,
                       UserRepository userRepo,
                       EmailVerificationTokenRepository tokenRepo,
                       PasswordEncoder encoder,
                       JwtService jwt,
                       AuthenticationManager authenticationManager) {
        this.emailService = emailService;
        this.userRepo = userRepo;
        this.tokenRepo = tokenRepo;
        this.encoder = encoder;
        this.jwt = jwt;
        this.authenticationManager = authenticationManager;
    }

    @Transactional
    public void register(RegisterRequest req) {
        if (userRepo.findByEmail(req.email).isPresent()) {
            throw new RuntimeException("Bu email zaten kayıtlı.");
        }

        User u = new User();
        u.setFirstName(req.firstName);
        u.setLastName(req.lastName);
        u.setNickname(req.nickname);
        u.setEmail(req.email);
        u.setPhone(req.phone);
        u.setPasswordHash(encoder.encode(req.password));
        u.setUserType("free");
        u.setIsActive(false); 
        userRepo.save(u);

        EmailVerificationToken t = new EmailVerificationToken();
        t.setUser(u);
        t.setToken(UUID.randomUUID());
        t.setExpiresAt(OffsetDateTime.now().plusDays(1));
        t.setUsed(false);
        tokenRepo.save(t);

        // Gerçek mail gönderimi
        emailService.sendVerificationEmail(u.getEmail(), t.getToken().toString());
    }

    public boolean verifyEmail(String tokenStr) {
        // ... (Bu kısım aynı kalabilir, değişen bir şey yok) ...
        try {
            UUID token = UUID.fromString(tokenStr);
            Optional<EmailVerificationToken> opt = tokenRepo.findByToken(token);
            if (opt.isEmpty()) return false;
            EmailVerificationToken t = opt.get();
            if (t.isUsed() || t.getExpiresAt().isBefore(OffsetDateTime.now())) return false;
            
            User u = t.getUser();
            u.setIsActive(true);
            t.setUsed(true);
            userRepo.save(u);
            tokenRepo.save(t);
            return true;
        } catch (Exception e) {
            return false;
        }
    }

    public AuthResponse login(LoginRequest req) {
        // Önce kullanıcıyı bul
        Optional<User> uOpt = userRepo.findByEmail(req.emailOrNickname);
        if (uOpt.isEmpty()) uOpt = userRepo.findByNickname(req.emailOrNickname);
        if (uOpt.isEmpty()) throw new RuntimeException("Kullanıcı bulunamadı");
        
        User u = uOpt.get();

        if (!u.getIsActive()) {
            throw new RuntimeException("Lütfen önce email adresinizi doğrulayın.");
        }

        // Spring Security ile doğrulama (Şifre kontrolünü o yapar)
        // Not: Burada UserDetails servisinde username olarak ID döndürdüğümüz için ID kullanıyoruz
        // Eğer karmaşık gelirse manuel şifre kontrolü de yapabilirsin (eski kodundaki gibi).
        // Ancak bu yöntem daha güvenlidir:
        try {
             // Manuel check (eski yöntem - şimdilik daha garanti çalışır senin yapında)
             if (!encoder.matches(req.password, u.getPasswordHash())) {
                 throw new RuntimeException("Şifre hatalı");
             }
        } catch (Exception e) {
            throw new RuntimeException("Giriş başarısız");
        }

        String token = jwt.generate(
                u.getUserId().toString(),
                Map.of("nickname", u.getNickname(), "role", u.getUserType())
        );

        return new AuthResponse(token, true);
    }
}