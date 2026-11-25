// Renderiza la interfaz a PNG sin DAW, para revisar contra la imagen aprobada.
// Uso: FlipperSnapshot salida.png [ancho=1024] [motor=8] [modo=0] [apagados=3,5]
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Help/I18n.h"
#include "Help/Help.h"
#include "Teach/Lessons.h"

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;

    const juce::String out = argc > 1 ? argv[1] : "flipper_snapshot.png";
    const int width        = argc > 2 ? juce::String (argv[2]).getIntValue() : 1024;
    const int engine       = argc > 3 ? juce::String (argv[3]).getIntValue() : 8;
    const int mode         = argc > 4 ? juce::String (argv[4]).getIntValue() : 0;
    const juce::String off = argc > 5 ? argv[5] : "";
    const bool tourMode = (mode >= 6 && mode <= 8) || mode >= 11;

    if (auto* lang = getenv ("FLIPPER_LANG")) flipper::i18n::setLanguage (juce::String (lang) == "en" ? flipper::i18n::Lang::en : flipper::i18n::Lang::es);
    if (mode == 10) // hoja de ayudas al pasar el mouse
    {
        const juce::StringArray keys { "macro_brightness", "ui:character_5", "dx7_op2_coarse", "pin:2:5", "minimoog_contour", "juno_chorus", "arp2600_c1_amt", "ui:generate" };
        flipper::FlipperLookAndFeel lf;
        juce::Image sheet (juce::Image::RGB, 640, 4 * 150, true);
        juce::Graphics g (sheet);
        g.fillAll (juce::Colour (0xff3a2a20));
        for (int i = 0; i < keys.size(); ++i)
        {
            const auto text = flipper::help::tooltip (keys[i]);
            auto r = lf.getTooltipBounds (text, { 0, 0 }, { 0, 0, 2000, 2000 });
            juce::Graphics::ScopedSaveState ss (g);
            g.setOrigin ((i % 2) * 320 + 8, (i / 2) * 150 + 8);
            lf.drawTooltip (g, text, r.getWidth(), r.getHeight());
        }
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile (out);
        f.deleteFile(); juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (sheet, os);
        std::printf ("OK %s\n", f.getFullPathName().toRawUTF8());
        return 0;
    }
    FlipperProcessor proc;
    if (mode == 9) // lista de todas las opciones de los parametros (para el diccionario)
    {
        juce::StringArray all;
        for (auto* p : proc.getParameters())
            if (auto* c = dynamic_cast<juce::AudioParameterChoice*> (p)) all.addArray (c->choices);
        all.removeDuplicates (false);
        juce::File (out).replaceWithText (all.joinIntoString ("\n"));
        return 0;
    }
    for (auto& token : juce::StringArray::fromTokens (tourMode ? juce::String() : off, ",", ""))
        if (token.isNotEmpty())
            if (auto* p = proc.state.getParameter (flipper::ids::engineOn (token.getIntValue())))
                p->setValueNotifyingHost (0.0f);

    if (mode == 4) // demo de audio del MINIMOOG: patch por defecto + 3 sonidos del generador (CLASICO)
    {
        const double sr = 48000.0; const int bs = 256;
        proc.prepareToPlay (sr, bs);
        for (int e = 0; e < flipper::numEngines; ++e)
            proc.state.getParameter (flipper::ids::engineOn (e))->setValueNotifyingHost (e == 0 ? 1.0f : 0.0f);
        proc.state.state.setProperty (flipper::ids::selectedEngine, 0, nullptr);
        proc.setGeneratorMode (flipper::SoundGenerator::Mode::classic);
        juce::AudioBuffer<float> all (2, (int) (sr * 16));
        all.clear();
        int writePos = 0;
        const int riff[] = { 36, 36, 48, 36, 39, 36, 43, 41 };
        const juce::uint32 seeds[] = { 0, 11, 23, 58 };
        for (int part = 0; part < 4; ++part)
        {
            if (seeds[part] != 0) proc.generateSound (seeds[part]);
            const int transpose = part == 0 ? 0 : 12 * (part % 2);
            for (int step = 0; step < 16 && writePos + bs * 44 < all.getNumSamples(); ++step)
            {
                const int note = riff[step % 8] + transpose;
                for (int b = 0; b < 44; ++b) // ~0.23 s por paso
                {
                    juce::AudioBuffer<float> buf (2, bs);
                    juce::MidiBuffer m;
                    if (b == 0) m.addEvent (juce::MidiMessage::noteOn (1, note, (juce::uint8) (90 + (step % 4 == 0 ? 30 : 0))), 0);
                    if (b == 30) m.addEvent (juce::MidiMessage::noteOff (1, note), 0);
                    proc.processBlock (buf, m);
                    for (int ch = 0; ch < 2; ++ch) all.copyFrom (ch, writePos, buf, ch, 0, bs);
                    writePos += bs;
                }
            }
        }
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile (out);
        f.deleteFile();
        std::unique_ptr<juce::AudioFormatWriter> w (juce::WavAudioFormat().createWriterFor (new juce::FileOutputStream (f), sr, 2, 24, {}, 0));
        w->writeFromAudioSampleBuffer (all, 0, writePos);
        w = nullptr;
        std::printf ("OK %s (%.1f s)\n", f.getFullPathName().toRawUTF8(), writePos / sr);
        return 0;
    }

    if (mode == 5) // demo: un acorde/frase por cada preset de fabrica
    {
        const double sr = 48000.0; const int bs = 256;
        proc.prepareToPlay (sr, bs);
        proc.presets.rescan();
        std::vector<const flipper::PresetManager::Preset*> list;
        for (auto& pr : proc.presets.all()) if (pr.factory) list.push_back (&pr);
        juce::AudioBuffer<float> all (2, (int) (sr * 2.4 * (double) list.size()) + bs);
        all.clear();
        int writePos = 0;
        for (auto* pr : list)
        {
            flipper::SoundGenerator::apply (proc.state, pr->values);
            const bool arp = proc.state.getParameter ("arp_on")->getValue() > 0.5f;
            const int notes[] = { 48, 55, 60, 63, 67 };
            const int blocks = (int) (2.4 * sr / bs);
            for (int b = 0; b < blocks; ++b)
            {
                juce::AudioBuffer<float> buf (2, bs);
                juce::MidiBuffer m;
                if (b == 0) for (int i = 0; i < (arp ? 3 : 5); ++i) m.addEvent (juce::MidiMessage::noteOn (1, notes[i] + (i == 0 && ! arp ? -12 : 0), (juce::uint8) 100), i);
                if (b == (int) (1.5 * sr / bs)) m.addEvent (juce::MidiMessage::allNotesOff (1), 0);
                proc.processBlock (buf, m);
                for (int ch = 0; ch < 2; ++ch) all.copyFrom (ch, writePos, buf, ch, 0, bs);
                writePos += bs;
            }
            std::printf ("%s\n", pr->name.toRawUTF8());
        }
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile (out);
        f.deleteFile();
        std::unique_ptr<juce::AudioFormatWriter> w (juce::WavAudioFormat().createWriterFor (new juce::FileOutputStream (f), sr, 2, 16, {}, 0));
        w->writeFromAudioSampleBuffer (all, 0, writePos);
        w = nullptr;
        std::printf ("OK %s (%.1f s)\n", f.getFullPathName().toRawUTF8(), writePos / sr);
        return 0;
    }

    if (mode == 2) // presets de muestra para ver el explorador
    {
        proc.presets.rescan();
        if (proc.presets.all().size() < 50)
        {
            const char* names[] = { "Bajo Kroko", "Coro Profeta", "Piter Brass", "Juna Pad", "Ojo Cinematico", "Trap Bell", "Arturito Keys", "Calavera Ruido" };
            for (int i = 0; i < 8; ++i)
            {
                proc.generateSound ((juce::uint32) (101 + i * 37));
                auto snap = flipper::SoundGenerator::capture (proc.state);
                proc.presets.save (names[i], flipper::PresetManager::categoryFor (snap), i % 3 == 0 ? "oscuro,808" : "", i % 2 == 0, snap);
            }
        }
        proc.setLocked ("minimoog", true);
        proc.setLocked ("macros", true);
        proc.setGeneratorMode (flipper::SoundGenerator::Mode::style);
        proc.generateSound ((juce::uint32) 4242);
    }

    proc.state.state.setProperty (flipper::ids::editorWidth, width, nullptr);
    std::unique_ptr<FlipperEditor> editor (static_cast<FlipperEditor*> (proc.createEditor()));
    editor->selectEngine (engine);
    editor->setMode ((flipper::UiMode) (mode >= 3 ? 0 : mode));
    if (mode == 2 || mode == 7) editor->setMode (flipper::UiMode::lab);
    editor->setSize (width, juce::roundToInt (width * 1.5));


    if (mode == 3) // hoja de cuadros de la animacion del personaje
    {
        auto& layer = editor->getAnimationLayer();
        layer.useManualClock (true);
        double t = 100000.0;
        layer.advance (t);
        layer.trigger (engine);
        const double times[] = { 120, 260, 330, 470, 620, 780, 950, 1120, 1300, 1500, 1750, 2050 };
        const int cols = 4, rows = 3, cw = 300, ch = 330;
        juce::Image sheet (juce::Image::RGB, cols * cw, rows * ch, true);
        juce::Graphics sg (sheet);
        static const float cx[] = { 288, 500, 707, 290, 507, 709, 232, 511, 753 };
        static const float cy[] = { 330, 300, 340, 600, 600, 610, 880, 940, 980 };
        const float zoom = (float) width / 1024.0f;
        for (int i = 0; i < 12; ++i)
        {
            while (t < 100000.0 + times[i]) { t += 16.0; layer.advance (t); }
            auto frame = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
            const auto src = juce::Rectangle<float> (cw * 1.6f, ch * 1.6f).withCentre ({ cx[engine], cy[engine] }) * zoom;
            sg.drawImage (frame, (i % cols) * cw, (i / cols) * ch, cw, ch,
                          (int) src.getX(), (int) src.getY(), (int) src.getWidth(), (int) src.getHeight());
            sg.setColour (juce::Colours::yellow);
            sg.drawText (juce::String ((int) times[i]) + " ms", (i % cols) * cw + 6, (i / cols) * ch + 4, 120, 18, juce::Justification::left);
        }
        juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile (out);
        f.deleteFile();
        juce::FileOutputStream os (f);
        juce::PNGImageFormat().writeImageToStream (sheet, os);
        std::printf ("OK %s\n", f.getFullPathName().toRawUTF8());
        editor = nullptr;
        return 0;
    }

    if (mode == 11) // MAESTRO: pantalla de pregunta (texto = argumento 5)
    {
        editor->openTeacher();
        editor->getTeacher().setQuery (juce::String::fromUTF8 (off.replace ("_", " ").toRawUTF8()));
    }
    if (mode == 12) // MAESTRO: paso de una receta ("receta:paso[:mostrame]")
    {
        auto parts = juce::StringArray::fromTokens (off, ":", "");
        if (auto* l = flipper::teach::find (parts[0]))
        {
            auto r = flipper::teach::search (parts.size() > 3 ? parts[3].replace ("_", " ") : juce::String());
            editor->getTeacher().startLesson (*l, r.modifiers);
            for (int i = 0; i < parts[1].getIntValue(); ++i) { editor->getTeacher().showMe(); editor->getTeacher().next(); }
            if (parts[2] == "1") editor->getTeacher().showMe();
        }
    }
    if (mode == 13) // MODULACION: DISENO con rutas y un arrastre en curso (motor = argumento 3)
    {
        editor->setMode (flipper::UiMode::design);
        auto& m = *proc.mod;
        const auto e = juce::String (flipper::engines()[(size_t) engine].id);
        int n = 0;
        for (auto* prm : proc.getParameters())
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (prm))
                if (rp->getParameterID().startsWith (e + "_") && m.canModulate (rp->getParameterID()) && n < 4)
                    m.setRoute (n == 0 ? flipper::ModMatrix::lfo1 : n == 1 ? flipper::ModMatrix::modA : n == 2 ? flipper::ModMatrix::lfo2 : flipper::ModMatrix::modC,
                                rp->getParameterID(), n % 2 ? 0.5f : -0.35f), ++n;
        m.setRoute (flipper::ModMatrix::modA, "macro_space", 0.4f);
        m.setRoute (flipper::ModMatrix::lfo1, e + "_cutoff", 0.3f);
        proc.state.getParameter ("mod_a")->setValueNotifyingHost (0.7f);
        juce::AudioBuffer<float> b (2, 512); juce::MidiBuffer none;
        for (int i = 0; i < 20; ++i) proc.processBlock (b, none);
        editor->getModOverlay().refresh();
        const auto target = editor->findControl (e + "_cutoff", true);
        if (! target.isEmpty() && off == "drag")
            editor->getModOverlay().dragUpdate (flipper::ModMatrix::modB, target.getCentre().toFloat() + juce::Point<float> (6, 4),
                                                e + "_cutoff", target);
    }
    if (mode == 14) // cartel de lectura sobre un control (id = argumento 5), con modulacion si hay
    {
        editor->setMode (flipper::UiMode::design);
        proc.mod->setRoute (flipper::ModMatrix::lfo1, off, 0.3f);
        editor->showReadoutFor (off);
    }
    if (mode >= 6 && mode <= 8) // un paso de un recorrido guiado (paso = argumento 5)
    {
        editor->setMode ((flipper::UiMode) (mode == 6 ? 0 : (mode == 7 ? 2 : 1)));
        editor->startTour();
        for (int i = 0; i < off.getIntValue(); ++i) editor->tourNext();
    }

    auto img = editor->createComponentSnapshot (editor->getLocalBounds(), true, 1.0f);
    juce::File f = juce::File::getCurrentWorkingDirectory().getChildFile (out);
    f.deleteFile();
    juce::FileOutputStream os (f);
    juce::PNGImageFormat png;
    const bool ok = os.openedOk() && png.writeImageToStream (img, os);
    std::printf ("%s %s (%dx%d)\n", ok ? "OK" : "ERROR", f.getFullPathName().toRawUTF8(), img.getWidth(), img.getHeight());
    editor = nullptr;
    return ok ? 0 : 1;
}
