#pragma once
#include <QDialog>

class QComboBox;
class QSpinBox;
class QFontComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;
class Settings;

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onBrowseDownloadLocation();
    void onClearCache();
    void onThemeChanged(int index);
    void onFontSizeChanged(int value);
    void onFontFamilyChanged(const QFont& font);
    void onApply();
    void onReset();

private:
    void setupUi();
    void loadSettings();
    void saveSettings();

    // Download
    QLineEdit* m_downloadLocationEdit;
    QPushButton* m_browseBtn;

    // Cache
    QLabel* m_cacheSizeLabel;
    QPushButton* m_clearCacheBtn;

    // Appearance
    QComboBox* m_themeCombo;        // light, dark, system
    QSpinBox* m_fontSizeSpin;
    QFontComboBox* m_fontCombo;

    // Upload defaults
    QComboBox* m_storageDestCombo;
    QComboBox* m_outputFormatCombo;
    QComboBox* m_cdnDomainCombo;

    QPushButton* m_applyBtn;
    QPushButton* m_resetBtn;
};
