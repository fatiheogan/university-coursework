package com.stackademy.stackademy.mail;

import org.springframework.beans.factory.annotation.Value;
import org.springframework.mail.SimpleMailMessage;
import org.springframework.mail.javamail.JavaMailSender;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;

@Service
public class EmailService {

    private final JavaMailSender mailSender;

    @Value("${app.url:http://localhost:3000}")
    private String appUrl;

    public EmailService(JavaMailSender mailSender) {
        this.mailSender = mailSender;
    }

    @Async
    public void sendVerificationEmail(String to, String token) {
        try {
            SimpleMailMessage message = new SimpleMailMessage();
            message.setFrom("stackademy@gmail.com");
            message.setTo(to);
            message.setSubject("Stackademy - Email Doğrulama");
            message.setText("Hesabını doğrulamak için tıkla: " + appUrl + "/verify-email?token=" + token);
            
            mailSender.send(message);
            System.out.println("Mail GÖNDERİLDİ -> " + to);
        } catch (Exception e) {
            System.err.println("Mail Hatası: " + e.getMessage());
        }
    }
}