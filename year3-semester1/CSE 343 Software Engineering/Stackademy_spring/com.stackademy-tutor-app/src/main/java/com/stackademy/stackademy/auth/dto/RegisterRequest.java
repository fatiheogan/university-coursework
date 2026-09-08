package com.stackademy.stackademy.auth.dto;

public class RegisterRequest {
  public String firstName;
  public String lastName;
  public String nickname;
  public String email;
  public String phone;
  public String password; // düz metin gelir; service içinde BCrypt ile hash'lenecek
}
