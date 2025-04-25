// PomodoroTimer.h
#ifndef POMODORO_TIMER_H
#define POMODORO_TIMER_H

#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/sound.h>
#include <wx/tglbtn.h> 
#include <wx/gauge.h>
#include <fstream>

// Enum untuk state timer
enum TimerState {
    READY,
    RUNNING_FOCUS,
    RUNNING_BREAK,
    PAUSED_FOCUS,
    PAUSED_BREAK
};

// Kelas utama aplikasi
class PomodoroApp : public wxApp {
public:
    virtual bool OnInit();
};

// Kelas untuk frame utama
class PomodoroFrame : public wxFrame {
public:
    PomodoroFrame(const wxString& title);
    virtual ~PomodoroFrame();

private:
    // Komponen GUI
    wxPanel* mainPanel;
    wxStaticText* timerDisplay;
    wxStaticText* stateDisplay;
    wxButton* startButton;
    wxButton* pauseButton;
    wxButton* resetButton;
    wxSlider* focusSlider;
    wxSlider* breakSlider;
    wxStaticText* focusValueText;
    wxStaticText* breakValueText;
    wxToggleButton* themeToggle;
    wxStatusBar* statusBar;
    wxGauge* progressBar;
    wxStaticText* statsText;
    
    // Dialog notifikasi
    wxDialog* notificationDialog;
    wxStaticText* notificationCountdown;
    int notificationCountdownSeconds;
    
    // Timer dan data
    wxTimer* timer;
    wxTimer* notificationTimer;
    TimerState timerState;
    int remainingSeconds;
    
    // Pengaturan
    int focusDuration;    // dalam menit
    int breakDuration;    // dalam menit
    bool darkMode;
    bool soundEnabled;
    
    // Statistik sederhana
    int completedSessions;
    
    // Sound
    wxSound* alarmSound;
    
    // Metode privat
    void CreateControls();
    void UpdateTimerDisplay();
    void ApplyTheme();
    void ShowNotificationDialog(bool isFocusCompleted);
    
    // Event handlers
    void OnStartTimer(wxCommandEvent& event);
    void OnPauseTimer(wxCommandEvent& event);
    void OnResetTimer(wxCommandEvent& event);
    void OnTimer(wxTimerEvent& event);
    void OnNotificationTimer(wxTimerEvent& event);
    void OnFocusSliderChange(wxCommandEvent& event);
    void OnBreakSliderChange(wxCommandEvent& event);
    void OnThemeToggle(wxCommandEvent& event);
    void OnSoundToggle(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    
    // File operations
    void SaveSettings();
    void LoadSettings();
    
    // Utility methods
    void StartFocusSession();
    void StartBreakSession();
    void CompleteSession();
    wxString FormatTimeDisplay(int seconds);
    
    DECLARE_EVENT_TABLE()
};

// IDs for controls
enum {
    ID_START_BUTTON = 1000,
    ID_PAUSE_BUTTON,
    ID_RESET_BUTTON,
    ID_FOCUS_SLIDER,
    ID_BREAK_SLIDER,
    ID_THEME_TOGGLE,
    ID_SOUND_TOGGLE,
    ID_TIMER,
    ID_NOTIFICATION_TIMER
};

#endif // POMODORO_TIMER_H