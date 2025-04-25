// PomodoroTimer.cpp
#include "PomodoroTimer.h"

// Implementasi event table
BEGIN_EVENT_TABLE(PomodoroFrame, wxFrame)
    EVT_BUTTON(ID_START_BUTTON, PomodoroFrame::OnStartTimer)
    EVT_BUTTON(ID_PAUSE_BUTTON, PomodoroFrame::OnPauseTimer)
    EVT_BUTTON(ID_RESET_BUTTON, PomodoroFrame::OnResetTimer)
    EVT_TIMER(ID_TIMER, PomodoroFrame::OnTimer)
    EVT_TIMER(ID_NOTIFICATION_TIMER, PomodoroFrame::OnNotificationTimer)
    EVT_SLIDER(ID_FOCUS_SLIDER, PomodoroFrame::OnFocusSliderChange)
    EVT_SLIDER(ID_BREAK_SLIDER, PomodoroFrame::OnBreakSliderChange)
    EVT_TOGGLEBUTTON(ID_THEME_TOGGLE, PomodoroFrame::OnThemeToggle)
    EVT_TOGGLEBUTTON(ID_SOUND_TOGGLE, PomodoroFrame::OnSoundToggle)
    EVT_CLOSE(PomodoroFrame::OnClose)
END_EVENT_TABLE()

// Implementasi kelas aplikasi
wxIMPLEMENT_APP(PomodoroApp);

bool PomodoroApp::OnInit() {
    PomodoroFrame* frame = new PomodoroFrame("Pomodoro Timer");
    frame->Show(true);
    return true;
}

// Implementasi konstruktor PomodoroFrame
PomodoroFrame::PomodoroFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(450, 350)) {
    
    // Inisialisasi data
    timerState = READY;
    remainingSeconds = 0;
    
    // Nilai default untuk pengaturan
    focusDuration = 25;
    breakDuration = 5;
    darkMode = false;
    soundEnabled = true;
    completedSessions = 0;
    notificationDialog = nullptr;
    
    // Mencoba memuat pengaturan dari file
    LoadSettings();
    
    // Inisialisasi timer
    timer = new wxTimer(this, ID_TIMER);
    notificationTimer = new wxTimer(this, ID_NOTIFICATION_TIMER);
    
    // Inisialisasi suara alarm
    alarmSound = new wxSound("alarm.wav");
    
    // Membuat GUI
    CreateControls();
    
    // Set icon and pos
    Centre();
    statusBar = CreateStatusBar();
    statusBar->SetStatusText("Status: Siap");
    
    // Terapkan tema
    ApplyTheme();
    
    // Set initial timer display
    remainingSeconds = focusDuration * 60;
    UpdateTimerDisplay();
}

// Destruktor
PomodoroFrame::~PomodoroFrame() {
    delete timer;
    delete notificationTimer;
    delete alarmSound;
    
    if (notificationDialog) {
        notificationDialog->Destroy();
    }
}

// Metode untuk membuat kontrol GUI - disederhanakan
void PomodoroFrame::CreateControls() {
    mainPanel = new wxPanel(this, wxID_ANY);
    
    // Warna latar belakang default
    mainPanel->SetBackgroundColour(wxColour(250, 250, 250));
    
    // Buat sizer utama
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    
    // ----- AREA TIMER -----
    // Timer display
    timerDisplay = new wxStaticText(mainPanel, wxID_ANY, "25:00", 
                                  wxDefaultPosition, wxDefaultSize,
                                  wxALIGN_CENTRE_HORIZONTAL);
    wxFont timerFont = timerDisplay->GetFont();
    timerFont.SetPointSize(42);
    timerFont.SetWeight(wxFONTWEIGHT_BOLD);
    timerDisplay->SetFont(timerFont);
    
    // State display
    stateDisplay = new wxStaticText(mainPanel, wxID_ANY, "SIAP", 
                                  wxDefaultPosition, wxDefaultSize,
                                  wxALIGN_CENTRE_HORIZONTAL);
    wxFont stateFont = stateDisplay->GetFont();
    stateFont.SetPointSize(12);
    stateDisplay->SetFont(stateFont);
    
    // Progress bar
    progressBar = new wxGauge(mainPanel, wxID_ANY, 100, 
                            wxDefaultPosition, wxSize(300, 15));
    
    mainSizer->Add(timerDisplay, 0, wxALIGN_CENTER | wxALL, 5);
    mainSizer->Add(stateDisplay, 0, wxALIGN_CENTER | wxALL, 3);
    mainSizer->Add(progressBar, 0, wxALIGN_CENTER | wxALL, 8);
    
    // ----- AREA TOMBOL -----
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    
    startButton = new wxButton(mainPanel, ID_START_BUTTON, "Start");
    pauseButton = new wxButton(mainPanel, ID_PAUSE_BUTTON, "Pause");
    resetButton = new wxButton(mainPanel, ID_RESET_BUTTON, "Reset");
    
    // Warna tombol
    
    pauseButton->Disable();
    resetButton->Disable();
    
    buttonSizer->Add(startButton, 0, wxALL, 5);
    buttonSizer->Add(pauseButton, 0, wxALL, 5);
    buttonSizer->Add(resetButton, 0, wxALL, 5);
    
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 10);
    
    // ----- AREA PENGATURAN -----
    wxStaticBox* settingsBox = new wxStaticBox(mainPanel, wxID_ANY, "Pengaturan");
    wxStaticBoxSizer* settingsSizer = new wxStaticBoxSizer(settingsBox, wxVERTICAL);
    
    // Grid untuk slider
    wxFlexGridSizer* sliderSizer = new wxFlexGridSizer(2, 3, 5, 10);
    sliderSizer->AddGrowableCol(1, 1);
    
    // Durasi fokus
    wxStaticText* focusLabel = new wxStaticText(mainPanel, wxID_ANY, "Durasi Fokus:");
    focusSlider = new wxSlider(mainPanel, ID_FOCUS_SLIDER, focusDuration, 15, 60,
                             wxDefaultPosition, wxSize(150, -1));
    focusValueText = new wxStaticText(mainPanel, wxID_ANY, 
                                    wxString::Format("%d menit", focusDuration));
    
    sliderSizer->Add(focusLabel, 0, wxALIGN_CENTER_VERTICAL);
    sliderSizer->Add(focusSlider, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
    sliderSizer->Add(focusValueText, 0, wxALIGN_CENTER_VERTICAL);
    
    // Durasi istirahat
    wxStaticText* breakLabel = new wxStaticText(mainPanel, wxID_ANY, "Durasi Istirahat:");
    breakSlider = new wxSlider(mainPanel, ID_BREAK_SLIDER, breakDuration, 5, 30,
                             wxDefaultPosition, wxSize(150, -1));
    breakValueText = new wxStaticText(mainPanel, wxID_ANY, 
                                    wxString::Format("%d menit", breakDuration));
    
    sliderSizer->Add(breakLabel, 0, wxALIGN_CENTER_VERTICAL);
    sliderSizer->Add(breakSlider, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
    sliderSizer->Add(breakValueText, 0, wxALIGN_CENTER_VERTICAL);
    
    settingsSizer->Add(sliderSizer, 0, wxEXPAND | wxALL, 5);
    
    // Toggle buttons
    wxBoxSizer* toggleSizer = new wxBoxSizer(wxHORIZONTAL);
    
    // Tema
    wxStaticText* themeLabel = new wxStaticText(mainPanel, wxID_ANY, "Tema:");
    themeToggle = new wxToggleButton(mainPanel, ID_THEME_TOGGLE, 
                                    darkMode ? "Dark Mode" : "Light Mode",
                                    wxDefaultPosition, wxSize(90, -1));
    themeToggle->SetValue(darkMode);
    themeToggle->SetBackgroundColour(wxColour(160, 160, 150));
    
    // Suara
    wxStaticText* soundLabel = new wxStaticText(mainPanel, wxID_ANY, "Suara:");
    wxToggleButton* soundToggle = new wxToggleButton(mainPanel, ID_SOUND_TOGGLE, 
                                    soundEnabled ? "ON" : "OFF",
                                    wxDefaultPosition, wxSize(70, -1));
    soundToggle->SetValue(soundEnabled);
    soundToggle->SetBackgroundColour(wxColour(160, 160, 150));
    
    toggleSizer->Add(themeLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    toggleSizer->Add(themeToggle, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 15);
    toggleSizer->Add(soundLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 5);
    toggleSizer->Add(soundToggle, 0, wxALIGN_CENTER_VERTICAL);
    
    settingsSizer->Add(toggleSizer, 0, wxALIGN_CENTER | wxALL, 5);
    
    mainSizer->Add(settingsSizer, 0, wxEXPAND | wxALL, 10);
    
    // ----- STATISTIK -----
    wxBoxSizer* statsSizer = new wxBoxSizer(wxHORIZONTAL);
    
    wxStaticText* statsLabel = new wxStaticText(mainPanel, wxID_ANY, "Statistik:");
    statsText = new wxStaticText(mainPanel, wxID_ANY, 
                               wxString::Format("Sesi selesai: %d", completedSessions));
    
    statsSizer->Add(statsLabel, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
    statsSizer->Add(statsText, 0, wxALIGN_CENTER_VERTICAL);
    
    mainSizer->Add(statsSizer, 0, wxALIGN_CENTER | wxALL, 10);
    
    mainPanel->SetSizer(mainSizer);
    mainSizer->Fit(this);
}

// Format waktu display
wxString PomodoroFrame::FormatTimeDisplay(int seconds) {
    int mins = seconds / 60;
    int secs = seconds % 60;
    return wxString::Format("%02d:%02d", mins, secs);
}

// Update timer display
void PomodoroFrame::UpdateTimerDisplay() {
    timerDisplay->SetLabel(FormatTimeDisplay(remainingSeconds));
    
    // Update state text dan progress bar
    wxString stateText;
    int progress = 0;
    
    switch (timerState) {
        case READY:
            stateText = "SIAP";
            progress = 0;
            break;
        case RUNNING_FOCUS:
            stateText = "FOKUS";
            progress = 100 - (remainingSeconds * 100 / (focusDuration * 60));
            break;
        case RUNNING_BREAK:
            stateText = "ISTIRAHAT";
            progress = 100 - (remainingSeconds * 100 / (breakDuration * 60));
            break;
        case PAUSED_FOCUS:
            stateText = "JEDA (FOKUS)";
            break;
        case PAUSED_BREAK:
            stateText = "JEDA (ISTIRAHAT)";
            break;
    }
    
    stateDisplay->SetLabel(stateText);
    progressBar->SetValue(progress);
    statusBar->SetStatusText(wxString::Format("Status: %s", stateText));
}

// Apply theme
void PomodoroFrame::ApplyTheme() {
    if (darkMode) {
        // Light mode
        mainPanel->SetBackgroundColour(wxColour(250, 250, 250));
        timerDisplay->SetForegroundColour(wxColour(20, 20, 20));
        stateDisplay->SetForegroundColour(wxColour(80, 80, 80));
        statsText->SetForegroundColour(wxColour(0, 0, 0));
        themeToggle->SetLabel("Light");
        startButton->SetBackgroundColour(wxColour(100, 150, 200));  // Biru
        startButton->SetForegroundColour(wxColour(255, 255, 255));
        
        pauseButton->SetBackgroundColour(wxColour(150, 150, 150));  // Abu-abu
        pauseButton->SetForegroundColour(wxColour(255, 255, 255));
        
        resetButton->SetBackgroundColour(wxColour(200, 100, 100));  // Merah
        resetButton->SetForegroundColour(wxColour(255, 255, 255));
} else {
        // krem mode
        mainPanel->SetBackgroundColour(wxColour(250, 245, 230)); // Warna krem yang lembut
        timerDisplay->SetForegroundColour(wxColour(70, 50, 30)); // Warna teks coklat gelap
        stateDisplay->SetForegroundColour(wxColour(100, 80, 60)); // Warna teks coklat medium
        statsText->SetForegroundColour(wxColour(80, 60, 40)); // Warna teks coklat
        themeToggle->SetLabel("krem");
        themeToggle->SetBackgroundColour(wxColour(160, 160, 150));

        startButton->SetBackgroundColour(wxColour(180, 140, 100)); // Tombol start dengan warna krem lebih gelap
        startButton->SetForegroundColour(wxColour(255, 255, 255));

        pauseButton->SetBackgroundColour(wxColour(160, 160, 150)); // Tombol pause dengan warna abu-abu krem
        pauseButton->SetForegroundColour(wxColour(255, 255, 255));

        resetButton->SetBackgroundColour(wxColour(180, 120, 120)); // Tombol reset dengan warna merah krem
        resetButton->SetForegroundColour(wxColour(255, 255, 255));
    }
    
    mainPanel->Refresh();
}

// Simplifikasi dialog notifikasi
void PomodoroFrame::ShowNotificationDialog(bool isFocusCompleted) {
    // Tutup dialog lama jika ada
    if (notificationDialog) {
        notificationDialog->Destroy();
        notificationDialog = nullptr;
    }
    
    wxString title, message;
    wxColour bgColor;
    
    if (isFocusCompleted) {
        title = "Waktu Fokus Selesai";
        message = wxString::Format(
            "Anda telah menyelesaikan sesi fokus %d menit.\n"
            "cobalah untuk minum dan beristirahat selama %d menit.",
            focusDuration, breakDuration);
        bgColor = wxColour(100, 150, 200); // Biru
        
        // Mulai countdown untuk notifikasi istirahat
        notificationCountdownSeconds = breakDuration * 60;
    } else {
        title = "Waktu Istirahat Selesai";
        message = wxString::Format(
            "Waktu istirahat %d menit Anda sudah selesai.\n"
            "Kembali ke sesi fokus selama %d menit.",
            breakDuration, focusDuration);
        bgColor = wxColour(130, 170, 220); // Biru lebih terang
        
        // Mulai countdown untuk kembali ke fokus
        notificationCountdownSeconds = 5;
    }
    
    // Buat dialog notifikasi sederhana
    notificationDialog = new wxDialog(this, wxID_ANY, title, 
                                    wxDefaultPosition, wxSize(320, 180),
                                    wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP);
    
    wxPanel* panel = new wxPanel(notificationDialog);
    panel->SetBackgroundColour(bgColor);
    
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    
    // Pesan teks
    wxStaticText* messageText = new wxStaticText(panel, wxID_ANY, message,
                                              wxDefaultPosition, wxDefaultSize,
                                              wxALIGN_CENTRE_HORIZONTAL);
    messageText->SetForegroundColour(wxColour(255, 255, 255));
    
    // Countdown text
    wxString countdownLabel = isFocusCompleted ? 
                              "Waktu istirahat berjalan:" : 
                              "Fokus dimulai dalam:";
    
    notificationCountdown = new wxStaticText(panel, wxID_ANY, 
                                          wxString::Format("%s %d detik", 
                                          countdownLabel, notificationCountdownSeconds),
                                          wxDefaultPosition, wxDefaultSize,
                                          wxALIGN_CENTRE_HORIZONTAL);
    notificationCountdown->SetForegroundColour(wxColour(255, 255, 255));
    
    sizer->Add(messageText, 0, wxALIGN_CENTER | wxALL | wxEXPAND, 15);
    sizer->Add(notificationCountdown, 0, wxALIGN_CENTER | wxALL, 10);
    
    panel->SetSizer(sizer);
    
    wxBoxSizer* dialogSizer = new wxBoxSizer(wxVERTICAL);
    dialogSizer->Add(panel, 1, wxEXPAND);
    notificationDialog->SetSizer(dialogSizer);
    
    notificationDialog->Centre();
    notificationDialog->Show();
    
    // Mulai timer notifikasi untuk countdown
    notificationTimer->Start(1000);
}

// Fungsi untuk memulai sesi fokus
void PomodoroFrame::StartFocusSession() {
    timerState = RUNNING_FOCUS;
    remainingSeconds = focusDuration * 60;
    
    // Update UI
    startButton->Disable();
    pauseButton->Enable();
    resetButton->Enable();
    
    // Update tampilan dan mulai timer
    UpdateTimerDisplay();
    timer->Start(1000);
}

// Fungsi untuk memulai sesi istirahat
void PomodoroFrame::StartBreakSession() {
    timerState = RUNNING_BREAK;
    remainingSeconds = breakDuration * 60;
    UpdateTimerDisplay();
    timer->Start(1000);
}

// Fungsi untuk menyelesaikan sesi
void PomodoroFrame::CompleteSession() {
    bool wasFocusSession = (timerState == RUNNING_FOCUS);
    
    if (wasFocusSession) {
        // Update statistik
        completedSessions++;
        statsText->SetLabel(wxString::Format("Sesi selesai: %d", completedSessions));
        SaveSettings();
    }
    
    // Mainkan suara alarm jika diaktifkan
    if (soundEnabled && alarmSound->IsOk()) {
        alarmSound->Play(wxSOUND_ASYNC);
    }
    
    // Tampilkan dialog notifikasi
    ShowNotificationDialog(wasFocusSession);
    
    if (wasFocusSession) {
        // Mulai istirahat setelah fokus selesai
        StartBreakSession();
    } else {
        // Hentikan timer setelah istirahat selesai
        timer->Stop();
    }
}

// Event handler: Timer start
void PomodoroFrame::OnStartTimer(wxCommandEvent& event) {
    if (timerState == READY) {
        StartFocusSession();
    } else if (timerState == PAUSED_FOCUS || timerState == PAUSED_BREAK) {
        // Resume timer
        timerState = (timerState == PAUSED_FOCUS) ? RUNNING_FOCUS : RUNNING_BREAK;
        startButton->Disable();
        pauseButton->Enable();
        timer->Start(1000);
        UpdateTimerDisplay();
    }
}

// Event handler: Timer pause
void PomodoroFrame::OnPauseTimer(wxCommandEvent& event) {
    if (timerState == RUNNING_FOCUS || timerState == RUNNING_BREAK) {
        // Pause timer
        timerState = (timerState == RUNNING_FOCUS) ? PAUSED_FOCUS : PAUSED_BREAK;
        timer->Stop();
        startButton->Enable();
        pauseButton->Disable();
        UpdateTimerDisplay();
    }
}

// Event handler: Timer reset
void PomodoroFrame::OnResetTimer(wxCommandEvent& event) {
    // Stop semua timer
    timer->Stop();
    if (notificationTimer->IsRunning()) {
        notificationTimer->Stop();
    }
    
    // Reset ke keadaan awal
    timerState = READY;
    remainingSeconds = focusDuration * 60;
    
    // Update UI
    startButton->Enable();
    pauseButton->Disable();
    resetButton->Disable();
    UpdateTimerDisplay();
    
    // Tutup dialog notifikasi jika ada
    if (notificationDialog) {
        notificationDialog->Destroy();
        notificationDialog = nullptr;
    }
}

// Event handler: Timer tick
void PomodoroFrame::OnTimer(wxTimerEvent& event) {
    if (remainingSeconds > 0) {
        remainingSeconds--;
        UpdateTimerDisplay();
    } else {
        CompleteSession();
    }
}

// Event handler: Notification timer tick - diperbaiki untuk kedua jenis notifikasi
void PomodoroFrame::OnNotificationTimer(wxTimerEvent& event) {
    if (notificationCountdownSeconds > 0) {
        notificationCountdownSeconds--;
        
        if (notificationDialog && notificationCountdown) {
            wxString countdownLabel;
            
            // Tentukan label berdasarkan state timer
            if (timerState == RUNNING_BREAK) {
                countdownLabel = "Waktu istirahat berjalan:";
            } else {
                countdownLabel = "Fokus dimulai dalam:";
            }
            
            notificationCountdown->SetLabel(wxString::Format("%s %d detik", 
                                                        countdownLabel, 
                                                        notificationCountdownSeconds));
        }
    } else {
        // Waktu habis
        notificationTimer->Stop();
        
        if (notificationDialog) {
            notificationDialog->Destroy();
            notificationDialog = nullptr;
        }
        
        // Jika istirahat selesai, mulai sesi fokus baru
        if (timerState != RUNNING_BREAK) {
            timerState = READY;
            StartFocusSession();
        }
    }
}

// Event handler: Fokus slider
void PomodoroFrame::OnFocusSliderChange(wxCommandEvent& event) {
    focusDuration = focusSlider->GetValue();
    focusValueText->SetLabel(wxString::Format("%d menit", focusDuration));
    
    if (timerState == READY) {
        remainingSeconds = focusDuration * 60;
        UpdateTimerDisplay();
    }
    
    SaveSettings();
}

// Event handler: Break slider
void PomodoroFrame::OnBreakSliderChange(wxCommandEvent& event) {
    breakDuration = breakSlider->GetValue();
    breakValueText->SetLabel(wxString::Format("%d menit", breakDuration));
    SaveSettings();
}

// Event handler: Theme toggle
void PomodoroFrame::OnThemeToggle(wxCommandEvent& event) {
    darkMode = themeToggle->GetValue();
    ApplyTheme();
    SaveSettings();
}

// Event handler: Sound toggle
void PomodoroFrame::OnSoundToggle(wxCommandEvent& event) {
    wxToggleButton* button = (wxToggleButton*)event.GetEventObject();
    soundEnabled = button->GetValue();
    button->SetLabel(soundEnabled ? "ON" : "OFF");
    SaveSettings();
}

// Event handler: Close window
void PomodoroFrame::OnClose(wxCloseEvent& event) {
    SaveSettings();
    event.Skip();
}

// Save settings
void PomodoroFrame::SaveSettings() {
    std::ofstream file("pomodoro_settings.txt");
    if (file.is_open()) {
        file << focusDuration << std::endl
             << breakDuration << std::endl
             << darkMode << std::endl
             << soundEnabled << std::endl
             << completedSessions << std::endl;
        file.close();
    }
}

// Load settings
void PomodoroFrame::LoadSettings() {
    std::ifstream file("pomodoro_settings.txt");
    if (file.is_open()) {
        file >> focusDuration >> breakDuration >> darkMode >> soundEnabled >> completedSessions;
        file.close();
    }
}