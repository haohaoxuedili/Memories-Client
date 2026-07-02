package com.mrcwoods.memories;

import android.content.SharedPreferences;
import android.graphics.Color;

public class ThemeConfig {
    public boolean darkMode;
    public int primaryButton;
    public int secondaryButton;
    public int background;
    public int primaryText;
    public int secondaryText;
    public float fontScale;
    public String fontFamily;
    public String presetName;

    public static ThemeConfig load(SharedPreferences prefs) {
        ThemeConfig theme = new ThemeConfig();
        theme.darkMode = prefs.getBoolean("theme_dark", false);
        theme.primaryButton = prefs.getInt("theme_primary_button", theme.darkMode ? AppConfig.DARK_PRIMARY_BUTTON : AppConfig.LIGHT_PRIMARY_BUTTON);
        theme.secondaryButton = prefs.getInt("theme_secondary_button", theme.darkMode ? AppConfig.DARK_SECONDARY_BUTTON : AppConfig.LIGHT_SECONDARY_BUTTON);
        theme.background = prefs.getInt("theme_background", theme.darkMode ? AppConfig.DARK_BACKGROUND : AppConfig.LIGHT_BACKGROUND);
        theme.primaryText = prefs.getInt("theme_primary_text", theme.darkMode ? AppConfig.DARK_PRIMARY_TEXT : AppConfig.LIGHT_PRIMARY_TEXT);
        theme.secondaryText = prefs.getInt("theme_secondary_text", theme.darkMode ? AppConfig.DARK_SECONDARY_TEXT : AppConfig.LIGHT_SECONDARY_TEXT);
        theme.fontScale = prefs.getFloat("theme_font_scale", 1.0f);
        theme.fontFamily = prefs.getString("theme_font_family", "sans");
        theme.presetName = prefs.getString("theme_preset_name", "default");
        return theme;
    }

    public void save(SharedPreferences prefs) {
        prefs.edit()
                .putBoolean("theme_dark", darkMode)
                .putInt("theme_primary_button", primaryButton)
                .putInt("theme_secondary_button", secondaryButton)
                .putInt("theme_background", background)
                .putInt("theme_primary_text", primaryText)
                .putInt("theme_secondary_text", secondaryText)
                .putFloat("theme_font_scale", fontScale)
                .putString("theme_font_family", fontFamily)
                .putString("theme_preset_name", presetName)
                .apply();
    }

    public void applyPreset(boolean dark) {
        darkMode = dark;
        primaryButton = dark ? AppConfig.DARK_PRIMARY_BUTTON : AppConfig.LIGHT_PRIMARY_BUTTON;
        secondaryButton = dark ? AppConfig.DARK_SECONDARY_BUTTON : AppConfig.LIGHT_SECONDARY_BUTTON;
        background = dark ? AppConfig.DARK_BACKGROUND : AppConfig.LIGHT_BACKGROUND;
        primaryText = dark ? AppConfig.DARK_PRIMARY_TEXT : AppConfig.LIGHT_PRIMARY_TEXT;
        secondaryText = dark ? AppConfig.DARK_SECONDARY_TEXT : AppConfig.LIGHT_SECONDARY_TEXT;
        presetName = "default";
    }

    public static int parseColorOrDefault(String value, int fallback) {
        try {
            return Color.parseColor(value.trim());
        } catch (Exception ignored) {
            return fallback;
        }
    }
}