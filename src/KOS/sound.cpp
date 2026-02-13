#include "KOS.h"

struct Track
{
    fs::FS *soundFS;
    String soundAddress;
} soundTrack;

struct soundNote
{
    int freq;
    float duration;
};

namespace KOS {
    TaskHandle_t soundPlayTask;

    void playSound(void *p)
    {
        pinMode(1, OUTPUT);

        std::vector<soundNote> commands;

        File f = soundTrack.soundFS->open(soundTrack.soundAddress);

        // USBSerial.println(f.size());

        while (f.available())
        {
            String s = f.readStringUntil('\n');
            int freq;
            float duration;

            std::vector<String> values = {""}; // Create vector of strings

            // We will hold every parameter splitted by space. example: {"v", "0.432", "2.334", "-1.411"}

            for (int i = 0; i < s.length(); i++)
            {
                if (s[i] == ',')
                {
                    values.back().trim();
                    values.push_back("");
                }
                else
                    values.back() += s[i];
            }

            if (s[0] == 'T')
            {
                freq = values[1].toInt();
                duration = values[2].toFloat();
                commands.push_back(soundNote{freq, duration});
            }
            else if (s[0] == 'D')
            {
                duration = values[1].toFloat();
                commands.push_back(soundNote{0, duration});
            }
        }

        f.close();

        ledcAttachChannel(1, 340, 10, 3);

        for (soundNote i : commands)
        {
            ledcWriteTone(1, i.freq);
            vTaskDelay(ceilf(i.duration));
        }

        ledcWriteTone(1, 0);

        pinMode(1, INPUT);

        soundPlayTask = NULL;

        vTaskDelete(NULL);
    }

    void playSound(fs::FS *fs, String filename)
    {
    #ifndef KOROBOCHKA3
        return;
    #endif
        if (soundPlayTask != NULL)
            vTaskDelete(soundPlayTask);
        soundTrack.soundFS = fs;
        soundTrack.soundAddress = filename;
        // USBSerial.println(filename);
        xTaskCreatePinnedToCore(playSound, "playSound", 16384, NULL, 4, &soundPlayTask, 1);
    }
}