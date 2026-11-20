#include "AnimationLayer.h"
#include "../UI/Theme.h"

namespace flipper::anim
{
using namespace juce;

//============================================================================== datos
namespace
{
enum Sprite
{
    sBall, sPillRed, sPillPurple, sPillOrange, sPillBlue, sCoinA, sCoinB, sCash, sVinyl, sBone, sHeart,
    sBurstOrange, sStarA, sStarB, sBurstBig, sStarGold, sSparkleMagenta, sExplosion, sFireBurst,
    sBoltA, sBoltB, sPuffA, sPuffB, sPuffC, sWisp, sSwirl, sArcWide, sArcSmall, sCount
};

const char* spriteNames[sCount] = {
    "ball_chrome_png", "pill_red_png", "pill_purple_png", "pill_orange_png", "pill_blue_png", "coin_a_png",
    "coin_b_png", "cash_png", "vinyl_png", "bone_b_png", "heart_png", "burst_orange_png", "star_a_png",
    "star_b_png", "burst_big_png", "star_gold_png", "sparkle_magenta_png", "explosion_png", "fire_burst_png",
    "bolt_a_png", "bolt_b_png", "puff_a_png", "puff_b_png", "puff_c_png", "wisp_png", "swirl_png",
    "arc_wide_png", "arc_small_png"
};

/** Datos por personaje (coordenadas del tablero 1024 x 1536). */
struct Def
{
    Rectangle<float> body;  // capa recortada del arte (elipse)
    Point<float> pivot;     // apoyo para squash & stretch
    Point<float> impact;    // donde pega la bola
    Point<float> a, b;      // puntos propios (ojos, boca, mano...)
    int bonus;
    const char* extra;
    double powerMs;
    double lateU;           // momento del evento fuerte del poder (0..1)
};

const std::array<Def, numEngines> defs {{
    /* MINIMOOG  la bestia   */ { { 200, 268, 175, 165 }, { 288, 433 }, { 338, 372 }, { 338, 372 }, { 288, 335 }, 4000, "",            850,  0.72 },
    /* PROPHET-5 el profeta  */ { { 392, 185, 215, 240 }, { 500, 425 }, { 500, 335 }, { 488, 256 }, { 528, 256 }, 5000, "",            800,  0.30 },
    /* JUPITER-8 el orbital  */ { { 618, 252, 178, 175 }, { 707, 427 }, { 690, 360 }, { 707, 340 }, { 707, 340 }, 4000, "",            900,  0.86 },
    /* JUNO      la wacha    */ { { 160, 488, 255, 265 }, { 288, 753 }, { 275, 665 }, { 342, 594 }, { 398, 560 }, 5500, "BONUS!",     1150, 0.60 },
    /* CS-80     el ojo      */ { { 428, 520, 158, 170 }, { 507, 690 }, { 507, 640 }, { 505, 612 }, { 505, 612 }, 6000, "EXTRA!",      950, 0.40 },
    /* DX7       el fumon    */ { { 602, 488, 215, 268 }, { 709, 756 }, { 690, 660 }, { 752, 600 }, { 705, 519 }, 3500, "",           1250, 0.30 },
    /* M1        el robot    */ { {  95, 782, 275, 215 }, { 232, 997 }, { 262, 895 }, { 247, 849 }, { 286, 849 }, 5000, "",            950, 0.45 },
    /* ARP 2600  el eterno   */ { { 372, 832, 278, 245 }, { 511, 1077 }, { 500, 975 }, { 512, 900 }, { 512, 952 }, 5500, "",           1250, 0.12 },
    /* VCS3      el espectro */ { { 648, 862, 210, 250 }, { 753, 1112 }, { 725, 950 }, { 725, 935 }, { 785, 1038 }, 5000, "MULTIPLIER!", 1250, 0.72 },
}};

const Point<float> ballStart { 512.0f, 1262.0f };
constexpr double launchMs = 220, hitMs = 170, bonusMs = 900, resetMs = 320, popupMs = 1150;

const Rectangle<float> junoHeadArea { 215, 478, 165, 150 };
const Point<float>     junoNeck     { 300, 615 };
const Rectangle<float> dxCapArea    { 640, 486, 132, 66 };
const Rectangle<float> arpSkullArea { 455, 848, 116, 108 };

float easeOutCubic (float t)   { t = jlimit (0.0f, 1.0f, t); return 1.0f - std::pow (1.0f - t, 3.0f); }
float easeInOutCubic (float t) { t = jlimit (0.0f, 1.0f, t); return t < 0.5f ? 4 * t * t * t : 1 - std::pow (-2 * t + 2, 3.0f) / 2; }
float easeOutBack (float t)    { t = jlimit (0.0f, 1.0f, t); const float c1 = 1.70158f, c3 = c1 + 1; return 1 + c3 * std::pow (t - 1, 3.0f) + c1 * std::pow (t - 1, 2.0f); }
float window (float u, float a, float b) { return jlimit (0.0f, 1.0f, (u - a) / (b - a)); }

Point<float> quad (Point<float> p0, Point<float> c, Point<float> p1, float t)
{
    const float it = 1 - t;
    return p0 * (it * it) + c * (2 * it * t) + p1 * (t * t);
}

AffineTransform around (Point<float> pivot, float sx, float sy, float rot, Point<float> d = {})
{
    return AffineTransform::translation (-pivot.x, -pivot.y).scaled (sx, sy).rotated (rot).translated (pivot.x + d.x, pivot.y + d.y);
}

Image makePatch (const Image& bg, Rectangle<float> area, float feather)
{
    const auto r = area.toNearestInt();
    Image img (Image::ARGB, r.getWidth(), r.getHeight(), true);
    {
        Graphics g (img);
        g.drawImage (bg, 0, 0, r.getWidth(), r.getHeight(), r.getX(), r.getY(), r.getWidth(), r.getHeight());
    }
    Image mask (Image::SingleChannel, r.getWidth(), r.getHeight(), true);
    {
        Graphics g (mask);
        const float w = (float) r.getWidth(), h = (float) r.getHeight();
        ColourGradient grad (Colours::white, w * 0.5f, h * 0.5f, Colours::transparentWhite, w, h * 0.5f, true);
        grad.addColour (1.0 - feather, Colours::white);
        g.addTransform (AffineTransform::scale (1.0f, h / w, w * 0.5f, h * 0.5f));
        g.setGradientFill (grad);
        g.fillRect (0.0f, h * 0.5f - w * 0.5f, w, w);
    }
    Image::BitmapData px (img, Image::BitmapData::readWrite);
    Image::BitmapData mk (mask, Image::BitmapData::readOnly);
    for (int y = 0; y < r.getHeight(); ++y)
        for (int x = 0; x < r.getWidth(); ++x)
            px.setPixelColour (x, y, px.getPixelColour (x, y).withMultipliedAlpha (mk.getPixelColour (x, y).getFloatAlpha()));
    return img;
}
}

//============================================================================== vida
AnimationLayer::AnimationLayer()
{
    setInterceptsMouseClicks (false, false);
    setSize (1024, 1536);
    for (auto* n : spriteNames) sprites.push_back (theme::image (n));
    buildPatches();
    now = lastAdvance = Time::getMillisecondCounterHiRes();
    ensureTimer();
}

AnimationLayer::~AnimationLayer() { stopTimer(); }

void AnimationLayer::buildPatches()
{
    const auto bg = theme::image ("main_playfield_jpg");
    for (int e = 0; e < numEngines; ++e)
        bodies[(size_t) e] = { makePatch (bg, defs[(size_t) e].body, 0.16f), defs[(size_t) e].body };
    junoHead = { makePatch (bg, junoHeadArea, 0.30f), junoHeadArea };
    dxCap    = { makePatch (bg, dxCapArea, 0.22f), dxCapArea };
    arpSkull = { makePatch (bg, arpSkullArea, 0.20f), arpSkullArea };
}

void AnimationLayer::setEnabled (bool shouldAnimate)
{
    enabled = shouldAnimate;
    if (! enabled)
    {
        for (auto& c : chars) c.active = false;
        for (auto& p : particles) p.alive = false;
        for (auto& p : popups) p.alive = false;
        repaint();
    }
    ensureTimer();
}

void AnimationLayer::setIdleEngine (int e)
{
    if (e == idleEngine) return;
    repaint (defs[(size_t) idleEngine].body.toNearestInt().expanded (8));
    idleEngine = e;
}

bool AnimationLayer::isAnimating (int e) const { return chars[(size_t) e].active; }

Phase AnimationLayer::phaseOf (int e) const
{
    if (! chars[(size_t) e].active) return Phase::idle;
    double u; return phaseAt (e, now - chars[(size_t) e].start, u);
}

void AnimationLayer::trigger (int e)
{
    if (! enabled || e < 0 || e >= numEngines) return;
    if (! manualClock) now = Time::getMillisecondCounterHiRes();

    auto& c = chars[(size_t) e];
    if (now < c.cooldownUntil) return;      // clicks repetidos: se ignoran hasta el cooldown

    clearOwner (e);                         // reactivacion limpia
    c = {};
    c.active = true;
    c.start = now;
    c.cooldownUntil = now + cooldownMs;
    c.ballDir = (defs[(size_t) e].impact - ballStart);
    c.ballDir /= jmax (1.0f, c.ballDir.getDistanceFromOrigin());
    c.seed = (uint32) rng.nextInt();
    if (onSfx) onSfx (Sfx::launch, e);
    ensureTimer();
}

void AnimationLayer::ensureTimer()
{
    if (manualClock) return;
    bool busy = false;
    for (auto& c : chars) busy |= c.active;
    for (auto& p : particles) busy |= p.alive;
    for (auto& p : popups) busy |= p.alive;

    if (! enabled && ! busy) { stopTimer(); return; }
    const int hz = busy ? 60 : 20;   // en reposo solo respira el personaje elegido
    if (! isTimerRunning() || getTimerInterval() != 1000 / hz) startTimerHz (hz);
}

void AnimationLayer::timerCallback()
{
    advance (Time::getMillisecondCounterHiRes());
    ensureTimer();
}

//============================================================================== fases
double AnimationLayer::phaseLength (int e, Phase p) const
{
    switch (p)
    {
        case Phase::launch: return launchMs;
        case Phase::hit:    return hitMs;
        case Phase::power:  return defs[(size_t) e].powerMs;
        case Phase::bonus:  return bonusMs;
        case Phase::reset:  return resetMs;
        case Phase::idle:   break;
    }
    return 0;
}

Phase AnimationLayer::phaseAt (int e, double t, double& u) const
{
    for (auto p : { Phase::launch, Phase::hit, Phase::power, Phase::bonus, Phase::reset })
    {
        const double len = phaseLength (e, p);
        if (t < len) { u = t / len; return p; }
        t -= len;
    }
    u = 1.0;
    return Phase::idle;
}

//============================================================================== simulacion
void AnimationLayer::advance (double nowMs)
{
    const float dt = (float) jlimit (0.0, 0.05, (nowMs - lastAdvance) / 1000.0);
    lastAdvance = now = nowMs;

    for (int e = 0; e < numEngines; ++e)
    {
        auto& c = chars[(size_t) e];
        if (! c.active) continue;
        const auto& d = defs[(size_t) e];
        double u = 0;
        const double t = now - c.start;
        const auto ph = phaseAt (e, t, u);

        if (ph == Phase::idle) { c.active = false; continue; }

        if (ph >= Phase::hit && ! c.sfxImpact)
        {
            c.sfxImpact = true;
            if (onSfx) onSfx (Sfx::impact, e);
            spawnRing (e, d.impact, Colours::white, 10, 260, 0.35f);
            burst (e, d.impact, 12, Colour (0xffffd25a), 260);
        }
        if (ph >= Phase::power && ! c.sfxPower)
        {
            c.sfxPower = true;
            if (onSfx) onSfx (Sfx::power, e);
            spawnPowerParticles (e);
        }
        if (ph == Phase::power && u >= d.lateU && ! c.spawnedLate) { c.spawnedLate = true; spawnLateParticles (e); }
        if (ph >= Phase::bonus && ! c.sfxBonus)
        {
            if (! c.spawnedLate) { c.spawnedLate = true; spawnLateParticles (e); }
            c.sfxBonus = true;
            if (onSfx) onSfx (Sfx::bonus, e);
            for (auto& p : popups)
                if (! p.alive)
                {
                    p.alive = true;
                    p.start = now;
                    p.text = "+" + String (d.bonus);
                    p.extra = d.extra;
                    float px = d.body.getCentreX() + (512.0f - d.body.getCentreX()) * 0.5f;
                    if (std::abs (d.body.getCentreX() - 512.0f) < 60.0f) px += 125.0f; // centrales: al costado
                    p.p = { jlimit (260.0f, 760.0f, px), jlimit (215.0f, 1180.0f, d.body.getY() + 6.0f) };
                    break;
                }
        }

        // emisiones continuas propias de cada poder
        const float fu = (float) u;
        if (ph == Phase::power)
        {
            switch (e)
            {
                case 1: // profeta: humo del cigarrillo
                    if (now - c.lastSpawn > 110) { c.lastSpawn = now; spawnSprite (e, sPuffC, { 546, 296 }, { rng.nextFloat() * 30 - 10, -60 }, 22, 1.1f, -10, 0.5f, 26, 0.6f); }
                    break;
                case 4: // ojo: carga de energia hacia el iris
                    if (fu < 0.4f)
                        for (int i = 0; i < 2; ++i)
                        {
                            const float ang = rng.nextFloat() * MathConstants<float>::twoPi;
                            const auto p = d.a + Point<float> (std::cos (ang), std::sin (ang)) * 95.0f;
                            spawnSpark (e, p, (d.a - p) * 2.6f, Colour (0xff9dff6a), 7, 0.38f);
                        }
                    break;
                case 5: // fumon: exhala
                    if (fu > 0.3f && fu < 0.55f && now - c.lastSpawn > 45)
                    {
                        c.lastSpawn = now;
                        spawnSprite (e, rng.nextBool() ? sPuffA : sPuffB, d.a, { -150 - rng.nextFloat() * 160, -60 - rng.nextFloat() * 90 },
                                     30, 1.2f, -15, rng.nextFloat() - 0.5f, 70, 1.2f);
                    }
                    break;
                case 8: // espectro: humo girando
                    if (now - c.lastSpawn > 90)
                    {
                        c.lastSpawn = now;
                        const float ang = rng.nextFloat() * MathConstants<float>::twoPi;
                        const auto p = d.a + Point<float> (std::cos (ang), std::sin (ang)) * 110.0f;
                        spawnSprite (e, sPuffA, p, Point<float> (-std::sin (ang), std::cos (ang)) * 160.0f + (d.a - p) * 0.8f,
                                     34, 0.9f, 0, 2.0f, -10, 0.5f);
                    }
                    break;
                default: break;
            }
        }
    }

    // humo en reposo del elegido (sutil)
    if (enabled && ! chars[(size_t) idleEngine].active && (idleEngine == 1 || idleEngine == 5)
        && now - lastIdleSpawn > 1500)
    {
        lastIdleSpawn = now;
        const Point<float> tip = idleEngine == 1 ? Point<float> (546, 296) : Point<float> (760, 598);
        spawnSprite (-2, sWisp, tip + Point<float> (0, -18), { 4, -22 }, 30, 2.4f, 0, 0.1f, 8, 0.2f);
    }

    // particulas
    for (auto& p : particles)
    {
        if (! p.alive) continue;
        p.age += dt;
        if (p.age >= p.life) { p.alive = false; continue; }
        p.v *= jmax (0.0f, 1.0f - p.drag * dt);
        p.v.y += p.gravity * dt;
        p.p += p.v * dt;
        p.rot += p.vrot * dt;
        p.size = jmax (1.0f, p.size + p.grow * dt);
    }
    for (auto& p : popups)
        if (p.alive && now - p.start > popupMs) p.alive = false;

    const auto dirty = dirtyArea();
    repaint (dirty.getUnion (lastDirty));
    lastDirty = dirty;
}

Rectangle<int> AnimationLayer::dirtyArea() const
{
    Rectangle<float> r;
    auto add = [&r] (Rectangle<float> a) { r = r.isEmpty() ? a : r.getUnion (a); };

    for (int e = 0; e < numEngines; ++e)
        if (chars[(size_t) e].active)
        {
            add (defs[(size_t) e].body.expanded (190));
            add (Rectangle<float> (ballStart.x - 30, ballStart.y - 30, 60, 60).getUnion (defs[(size_t) e].body));
            if (e == 4) add ({ 430, 600, 160, 680 }); // rayo del ojo
        }
    if (enabled) add (defs[(size_t) idleEngine].body.expanded (10));
    for (auto& p : particles)
        if (p.alive) add (Rectangle<float> (p.size * 2.5f + 8, p.size * 2.5f + 8).withCentre (p.p));
    for (auto& p : popups)
        if (p.alive) add (Rectangle<float> (300, 170).withCentre (p.p.translated (0, -30)));
    return r.getSmallestIntegerContainer().getIntersection (getLocalBounds());
}

//============================================================================== particulas
AnimationLayer::Particle* AnimationLayer::spawn (int owner)
{
    for (auto& p : particles)
        if (! p.alive) { p = {}; p.alive = true; p.owner = owner; return &p; }
    return nullptr; // pool lleno: no se acumulan
}

void AnimationLayer::spawnSprite (int owner, int sprite, Point<float> pos, Point<float> v, float size, float life,
                                  float gravity, float vrot, float grow, float drag)
{
    if (auto* p = spawn (owner))
    {
        p->kind = 0; p->sprite = sprite; p->p = pos; p->v = v; p->size = size; p->life = life;
        p->gravity = gravity; p->vrot = vrot; p->grow = grow; p->drag = drag; p->rot = rng.nextFloat() * 0.6f - 0.3f;
    }
}

void AnimationLayer::spawnSpark (int owner, Point<float> pos, Point<float> v, Colour col, float size, float life, float gravity)
{
    if (auto* p = spawn (owner))
    {
        p->kind = 1; p->p = pos; p->v = v; p->colour = col; p->size = size; p->life = life; p->gravity = gravity; p->drag = 1.5f;
    }
}

void AnimationLayer::spawnRing (int owner, Point<float> pos, Colour col, float size, float grow, float life)
{
    if (auto* p = spawn (owner))
    {
        p->kind = 2; p->p = pos; p->colour = col; p->size = size; p->grow = grow; p->life = life;
    }
}

void AnimationLayer::burst (int owner, Point<float> pos, int count, Colour col, float speed)
{
    for (int i = 0; i < count; ++i)
    {
        const float ang = rng.nextFloat() * MathConstants<float>::twoPi;
        const float s = speed * (0.4f + rng.nextFloat() * 0.8f);
        spawnSpark (owner, pos, { std::cos (ang) * s, std::sin (ang) * s }, col, 4 + rng.nextFloat() * 5, 0.35f + rng.nextFloat() * 0.3f, 200);
    }
}

void AnimationLayer::clearOwner (int owner)
{
    for (auto& p : particles)
        if (p.owner == owner) p.alive = false;
}

void AnimationLayer::spawnPowerParticles (int e)
{
    const auto& d = defs[(size_t) e];
    switch (e)
    {
        case 8: // espectro: absorbe -> distorsion
            spawnRing (e, d.a, Colour (0xff9a3cff), 150, -200, 0.6f);
            break;
        case 6: // robot: chispas de sobrecarga
            burst (e, d.a.translated (20, 0), 8, Colour (0xffffb020), 160);
            break;
        default: break;
    }
}

void AnimationLayer::spawnLateParticles (int e)
{
    const auto& d = defs[(size_t) e];
    auto rnd = [this] (float a, float b) { return a + rng.nextFloat() * (b - a); };

    switch (e)
    {
        case 0: // la bestia escupe monedas
            for (int i = 0; i < 11; ++i)
                spawnSprite (e, i % 5 == 0 ? sCash : (i % 2 ? sCoinA : sCoinB), d.a,
                             { rnd (90, 330), rnd (-420, -180) }, rnd (36, 48), 1.3f, 950, rnd (-8, 8));
            burst (e, d.a, 14, Colour (0xffffd25a), 300);
            break;

        case 1: // explosion astral
            for (int i = 0; i < 3; ++i)
                spawnRing (e, d.body.getCentre(), i == 1 ? Colour (0xff7fc8ff) : Colour (0xffffd25a), 20, 380 + i * 120.0f, 0.7f + i * 0.12f);
            for (int i = 0; i < 16; ++i)
            {
                const float ang = MathConstants<float>::twoPi * (float) i / 16.0f;
                spawnSprite (e, i % 2 ? sStarA : sStarB, d.body.getCentre(), { std::cos (ang) * 340, std::sin (ang) * 340 }, 26, 0.75f, 0, 4, -10, 1.5f);
            }
            break;

        case 2: // explosion circular
            spawnRing (e, d.a, Colour (0xffffd25a), 30, 520, 0.55f);
            spawnRing (e, d.a, Colour (0xff7fc8ff), 20, 380, 0.65f);
            spawnSprite (e, sExplosion, d.a, {}, 120, 0.45f, 0, 1, 260);
            burst (e, d.a, 22, Colour (0xffff8a20), 380);
            break;

        case 3: // la wacha atrapa la pastilla
            spawnRing (e, d.a, Colours::white, 8, 320, 0.4f);
            for (int i = 0; i < 18; ++i)
            {
                const float ang = rnd (-3.1f, 0.2f);
                const int spr = i % 4 == 0 ? sHeart : (i % 3 == 0 ? sSparkleMagenta : sStarGold);
                spawnSprite (e, spr, d.a, { std::cos (ang) * rnd (160, 380), std::sin (ang) * rnd (160, 380) }, rnd (18, 30), 0.9f, 260, rnd (-5, 5), 0, 1.2f);
            }
            break;

        case 4: // rayo del ojo
            spawnRing (e, d.a, Colour (0xffb8ff7a), 20, 420, 0.5f);
            for (int i = 0; i < 16; ++i)
                spawnSpark (e, { d.a.x + rnd (-14, 14), rnd (650, 1220) }, { rnd (-160, 160), rnd (-90, 40) }, Colour (0xffe8ff9a), rnd (4, 8), 0.5f);
            break;

        case 5: // onda expansiva del humo
            spawnRing (e, d.a, Colour (0xffe8dcc8), 16, 360, 0.6f);
            break;

        case 6: // descarga electrica
            for (int i = 0; i < 24; ++i)
            {
                const float ang = rnd (0, MathConstants<float>::twoPi);
                spawnSpark (e, d.a.translated (20, 0), { std::cos (ang) * rnd (120, 420), std::sin (ang) * rnd (120, 420) },
                            i % 2 ? Colour (0xff7fe7ff) : Colour (0xffffe066), rnd (4, 8), rnd (0.3f, 0.6f), 150);
            }
            for (int i = 0; i < 3; ++i)
                spawnSprite (e, i % 2 ? sBoltA : sBoltB, d.a.translated (20, 0), { rnd (-260, 260), rnd (-260, -60) }, 40, 0.5f, 400, rnd (-6, 6));
            break;

        case 7: // el craneo se va: particulas doradas y huesos
            for (int i = 0; i < 14; ++i)
                spawnSpark (e, d.b, { rnd (-260, 260), rnd (-320, -60) }, Colour (0xffffcf4a), rnd (4, 7), rnd (0.5f, 0.9f), 600);
            for (int i = 0; i < 5; ++i)
                spawnSprite (e, sBone, d.b, { rnd (-240, 240), rnd (-380, -160) }, rnd (26, 38), 1.2f, 900, rnd (-9, 9));
            break;

        case 8: // la bola vuelve con multiplicador
            spawnRing (e, d.a, Colour (0xffc070ff), 14, 360, 0.5f);
            burst (e, d.a, 20, Colour (0xffd890ff), 340);
            break;

        default: break;
    }
}

//============================================================================== bola
Point<float> AnimationLayer::ballPosition (int e, double t, float& scale, float& alpha) const
{
    const auto& d = defs[(size_t) e];
    const auto& c = chars[(size_t) e];
    double u = 0;
    const auto ph = phaseAt (e, t, u);
    const float fu = (float) u;
    alpha = 1.0f;

    if (ph == Phase::launch)
    {
        const Point<float> mid = (ballStart + d.impact) * 0.5f;
        const float side = d.impact.x < 512.0f ? -1.0f : 1.0f;
        const Point<float> ctrl = mid + Point<float> (side * 110.0f, 40.0f);
        scale = 0.72f + 0.18f * fu;
        return quad (ballStart, ctrl, d.impact, easeOutCubic (fu * 0.85f + 0.15f * fu * fu));
    }
    if (ph == Phase::hit)
    {
        scale = 0.9f;
        if (e == 0) { scale = 0.9f - 0.6f * fu; alpha = 1 - fu; return d.a; }        // a la boca
        if (e == 2 || e == 8) return d.impact;                                      // queda para el poder
        alpha = 1.0f - fu;
        return d.impact - c.ballDir * (70.0f * easeOutCubic (fu)) + Point<float> (c.ballDir.y, -c.ballDir.x) * 40.0f * fu;
    }
    if (ph == Phase::power)
    {
        if (e == 2) // orbita alrededor de PITER
        {
            const float ang = fu * MathConstants<float>::twoPi * 1.5f + 2.6f;
            const float rot = -0.25f;
            const Point<float> o (std::cos (ang) * 118.0f, std::sin (ang) * 40.0f);
            scale = 0.82f + 0.12f * std::sin (ang);
            alpha = fu > 0.86f ? 0.0f : 1.0f;
            return d.a + Point<float> (o.x * std::cos (rot) - o.y * std::sin (rot), o.x * std::sin (rot) + o.y * std::cos (rot));
        }
        if (e == 8) // absorbida en espiral y devuelta
        {
            if (fu < 0.42f)
            {
                const float v = fu / 0.42f;
                const float ang = v * MathConstants<float>::twoPi * 1.75f;
                const float rad = (1 - v) * 70.0f;
                scale = 0.9f * (1 - v * 0.75f);
                alpha = 1 - v * 0.6f;
                return d.a + Point<float> (std::cos (ang), std::sin (ang)) * rad;
            }
            if (fu > 0.72f)
            {
                const float v = (fu - 0.72f) / 0.28f;
                scale = 1.0f;
                return d.a + Point<float> (-260.0f, -170.0f) * easeOutCubic (v);
            }
            alpha = 0.0f;
            return d.a;
        }
    }
    if (e == 8 && ph == Phase::bonus)
    {
        scale = 1.0f; alpha = 1.0f - fu;
        return d.a + Point<float> (-260.0f, -170.0f) + Point<float> (-60.0f, -40.0f) * fu;
    }
    alpha = 0.0f;
    scale = 1.0f;
    return d.impact;
}

//============================================================================== transformaciones
AffineTransform AnimationLayer::bodyTransform (int e, Phase ph, float u, double t) const
{
    const auto& d = defs[(size_t) e];
    const auto& c = chars[(size_t) e];

    if (ph == Phase::hit)
    {
        const float k = std::sin (MathConstants<float>::pi * u) * (1 - u * 0.3f);
        const float side = c.ballDir.x < 0 ? -1.0f : 1.0f;
        return around (d.pivot, 1 + 0.10f * k, 1 - 0.12f * k, 0.06f * k * side, c.ballDir * (9.0f * k));
    }
    if (ph == Phase::bonus)
    {
        const float k = std::sin (u * MathConstants<float>::pi * 3) * (1 - u);
        return around (d.pivot, 1 - 0.03f * k, 1 + 0.05f * k, 0);
    }
    if (ph != Phase::power) return {};

    switch (e)
    {
        case 0: // mordidas y sacudida de cabeza
        {
            if (u < 0.4f)
            {
                const float ch = std::abs (std::sin (u / 0.4f * MathConstants<float>::twoPi));
                return around (d.pivot, 1 + 0.05f * ch, 1 - 0.10f * ch, 0);
            }
            const float v = window (u, 0.4f, 0.8f);
            return around (d.b, 1, 1, 0.11f * std::sin (v * MathConstants<float>::pi * 6) * (1 - v));
        }
        case 1: // pose de profeta: se eleva
        {
            const float k = std::sin (MathConstants<float>::pi * u);
            return around (d.pivot, 1 + 0.04f * k, 1 + 0.04f * k, 0, { 0, -9.0f * k });
        }
        case 2: // giro completo del planeta
            return around (d.a, 1 + 0.05f * std::sin (MathConstants<float>::pi * u), 1 + 0.05f * std::sin (MathConstants<float>::pi * u),
                           MathConstants<float>::twoPi * easeInOutCubic (u));
        case 3: // rebote canchero despues de atrapar la pastilla
        {
            if (u < 0.1f) return around (d.pivot, 1, 1, -0.03f * std::sin (u / 0.1f * MathConstants<float>::pi));
            const float v = window (u, 0.6f, 1.0f);
            const float k = std::sin (v * MathConstants<float>::twoPi) * (1 - v);
            return around (d.pivot, 1 - 0.04f * k, 1 + 0.06f * k, 0.02f * k);
        }
        case 4: // vibra mientras carga, golpe al disparar
        {
            if (u > 0.15f && u < 0.4f)
            {
                Random r ((int64) (t / 16.0) + 4);
                return AffineTransform::translation (r.nextFloat() * 3 - 1.5f, r.nextFloat() * 3 - 1.5f);
            }
            const float v = window (u, 0.4f, 0.55f);
            const float k = std::sin (v * MathConstants<float>::pi);
            return around (d.pivot, 1 + 0.06f * k, 1 - 0.05f * k, 0);
        }
        case 5: // pitada, exhala y reaccion
        {
            if (u < 0.3f) { const float k = std::sin (u / 0.3f * MathConstants<float>::halfPi); return around (d.pivot, 1 + 0.035f * k, 1 + 0.035f * k, 0); }
            if (u < 0.42f) { const float k = 1 - window (u, 0.3f, 0.42f); return around (d.pivot, 1 + 0.035f * k, 1 + 0.035f * k, 0); }
            const float v = window (u, 0.38f, 0.85f);
            return around (d.pivot, 1, 1, 0.07f * std::sin (v * MathConstants<float>::pi * 5) * (1 - v));
        }
        case 6: // sobrecarga: vibracion de la carcasa
        {
            const float amp = 3.8f * std::sin (MathConstants<float>::pi * window (u, 0.12f, 0.8f));
            Random r ((int64) (t / 16.0) + 6);
            return AffineTransform::translation ((r.nextFloat() * 2 - 1) * amp, (r.nextFloat() * 2 - 1) * amp);
        }
        case 7: // recoil al soltar el craneo y al volver
        {
            const float k1 = std::sin (MathConstants<float>::pi * window (u, 0.0f, 0.15f));
            const float k2 = std::sin (MathConstants<float>::pi * window (u, 0.85f, 1.0f));
            return around (d.pivot, 1 + 0.03f * (k1 + k2), 1 - 0.04f * (k1 + k2), 0);
        }
        case 8: // se encoge al absorber, retrocede al devolver
        {
            const float k1 = std::sin (MathConstants<float>::pi * window (u, 0.0f, 0.42f));
            const float k2 = std::sin (MathConstants<float>::pi * window (u, 0.72f, 0.95f));
            return around (d.pivot, 1 - 0.04f * k1 + 0.05f * k2, 1 - 0.04f * k1 - 0.06f * k2, 0, { 8.0f * k2, 0 });
        }
        default: break;
    }
    return {};
}

//============================================================================== dibujo
void AnimationLayer::paintPatch (Graphics& g, const Patch& p, const AffineTransform& t, float alpha)
{
    Graphics::ScopedSaveState s (g);
    g.setOpacity (alpha);
    g.drawImageTransformed (p.image, AffineTransform::translation (p.area.getX(), p.area.getY()).followedBy (t));
}

void AnimationLayer::paintHole (Graphics& g, Rectangle<float> r, Colour col, float alpha)
{
    if (alpha <= 0.0f) return;
    const auto c = r.getCentre();
    ColourGradient grad (col.withAlpha (alpha), c.x, c.y, col.withAlpha (0.0f), c.x + r.getWidth() * 0.55f, c.y, true);
    grad.addColour (0.6, col.withAlpha (alpha * 0.9f));
    Graphics::ScopedSaveState s (g);
    g.addTransform (AffineTransform::scale (1.0f, r.getHeight() / r.getWidth(), c.x, c.y));
    g.setGradientFill (grad);
    g.fillEllipse (c.x - r.getWidth() * 0.55f, c.y - r.getWidth() * 0.55f, r.getWidth() * 1.1f, r.getWidth() * 1.1f);
}

void AnimationLayer::paint (Graphics& g)
{
    if (! enabled && lastDirty.isEmpty()) return;
    g.setImageResamplingQuality (Graphics::mediumResamplingQuality);
    {
        RectangleList<int> area;   // entre los paneles: no se tapan los controles
        area.add ({ 182, 150, 640, 590 });
        area.add ({ 182, 740, 676, 576 });
        g.reduceClipRegion (area);
    }

    if (enabled && ! chars[(size_t) idleEngine].active)
        paintIdle (g, idleEngine);

    for (int e = 0; e < numEngines; ++e)
        if (chars[(size_t) e].active)
            paintCharacter (g, e, now - chars[(size_t) e].start);

    // particulas
    for (auto& p : particles)
    {
        if (! p.alive) continue;
        const float a = jmin (1.0f, p.age / (0.1f * p.life)) * jmin (1.0f, (p.life - p.age) / (0.4f * p.life));
        if (p.kind == 0 && p.sprite >= 0)
        {
            const auto& img = sprites[(size_t) p.sprite];
            if (! img.isValid()) continue;
            const float s = p.size / (float) jmax (img.getWidth(), img.getHeight());
            Graphics::ScopedSaveState ss (g);
            g.setOpacity (a);
            g.drawImageTransformed (img, AffineTransform::translation (-img.getWidth() * 0.5f, -img.getHeight() * 0.5f)
                                             .scaled (s).rotated (p.rot).translated (p.p.x, p.p.y));
        }
        else if (p.kind == 1)
        {
            g.setGradientFill (ColourGradient (p.colour.withAlpha (a), p.p.x, p.p.y, p.colour.withAlpha (0.0f), p.p.x + p.size * 1.8f, p.p.y, true));
            g.fillEllipse (Rectangle<float> (p.size * 3.6f, p.size * 3.6f).withCentre (p.p));
            g.setColour (Colours::white.withAlpha (a));
            g.fillEllipse (Rectangle<float> (p.size * 0.7f, p.size * 0.7f).withCentre (p.p));
        }
        else if (p.kind == 2)
        {
            const float k = 1 - p.age / p.life;
            g.setColour (p.colour.withAlpha (a * 0.35f));
            g.drawEllipse (Rectangle<float> (p.size * 2, p.size * 2).withCentre (p.p), 9.0f * k + 2);
            g.setColour (p.colour.withAlpha (a));
            g.drawEllipse (Rectangle<float> (p.size * 2, p.size * 2).withCentre (p.p), 3.0f * k + 1);
        }
    }

    // bolas en vuelo
    for (int e = 0; e < numEngines; ++e)
    {
        if (! chars[(size_t) e].active) continue;
        const double t = now - chars[(size_t) e].start;
        float sc = 1, al = 1, sc0 = 1, al0 = 1;
        const auto p = ballPosition (e, t, sc, al);
        const auto p0 = ballPosition (e, jmax (0.0, t - 24.0), sc0, al0);
        if (al > 0.01f) paintBall (g, p, sc, al, p - p0);
    }

    for (auto& p : popups)
        if (p.alive) paintPopup (g, p);
}

void AnimationLayer::paintIdle (Graphics& g, int e)
{
    const auto& d = defs[(size_t) e];
    const float t = (float) (now / 1000.0);
    const float breath = std::sin (t * MathConstants<float>::twoPi / 3.2f);

    auto tr = around (d.pivot, 1 + 0.004f * breath, 1 + 0.009f * breath, 0);
    if (e == 2) tr = around (d.a, 1, 1, 0.02f * std::sin (t * 1.3f));                 // PITER se balancea
    if (e == 3) tr = around (d.pivot, 1, 1 + 0.008f * breath, 0.006f * std::sin (t * 1.1f));
    paintPatch (g, bodies[(size_t) e], tr);

    if (e == 4) // iris que late
    {
        const float k = 0.5f + 0.5f * std::sin (t * 2.4f);
        g.setGradientFill (ColourGradient (Colour (0xff9dff6a).withAlpha (0.25f + 0.25f * k), d.a.x, d.a.y,
                                           Colour (0x009dff6a), d.a.x + 26, d.a.y, true));
        g.fillEllipse (Rectangle<float> (52, 52).withCentre (d.a));
    }
    if (e == 6) // LEDs del robot
    {
        Random r ((int64) (now / 180.0));
        for (auto p : { d.a, d.b })
        {
            const float k = 0.35f + 0.4f * r.nextFloat();
            g.setGradientFill (ColourGradient (Colour (0xffff7a20).withAlpha (k), p.x, p.y, Colour (0x00ff7a20), p.x + 18, p.y, true));
            g.fillEllipse (Rectangle<float> (36, 36).withCentre (p));
        }
    }
}

void AnimationLayer::paintCharacter (Graphics& g, int e, double t)
{
    const auto& d = defs[(size_t) e];
    double du = 0;
    const auto ph = phaseAt (e, t, du);
    const float u = (float) du;
    if (ph == Phase::launch) { paintIdle (g, e); return; }

    // tiempo relativo al poder (puede pasar de 1 en el bonus, para colas de efectos)
    const double powerStart = launchMs + hitMs;
    const float pu = (float) ((t - powerStart) / d.powerMs);

    const auto tr = bodyTransform (e, ph, u, t);
    const bool moving = ph == Phase::hit || ph == Phase::power || ph == Phase::bonus;

    // sombra bajo la capa para que el original no se vea doble
    paintHole (g, d.body.reduced (d.body.getWidth() * 0.08f), Colours::black, moving ? 0.38f : 0.0f);

    if (e == 8 && ph == Phase::power) // distorsion oscura detras del espectro
    {
        const float k = std::sin (MathConstants<float>::pi * jlimit (0.0f, 1.0f, u));
        g.setGradientFill (ColourGradient (Colour (0xff1a0626).withAlpha (0.75f * k), d.a.x, d.a.y, Colour (0x001a0626), d.a.x + 170, d.a.y, true));
        g.fillEllipse (Rectangle<float> (340, 340).withCentre (d.a));
        if (const auto& sw = sprites[sSwirl]; sw.isValid())
        {
            Graphics::ScopedSaveState s (g);
            g.setOpacity (0.8f * k);
            const float sc = 300.0f / sw.getWidth();
            g.drawImageTransformed (sw, AffineTransform::translation (-sw.getWidth() * 0.5f, -sw.getHeight() * 0.5f)
                                            .scaled (sc, sc * 0.8f).rotated (-u * 9.0f).translated (d.a.x, d.a.y + 20));
        }
    }

    // cuerpo (con glitch para el robot)
    if (e == 6 && ph == Phase::power && u > 0.45f && u < 0.66f)
    {
        Random r ((int64) (t / 40.0));
        const auto area = d.body;
        const int slices = 9;
        for (int i = 0; i < slices; ++i)
        {
            const float y0 = area.getY() + area.getHeight() * (float) i / slices;
            Graphics::ScopedSaveState s (g);
            g.reduceClipRegion (Rectangle<float> (area.getX() - 30, y0, area.getWidth() + 60, area.getHeight() / slices + 1).toNearestInt());
            paintPatch (g, bodies[(size_t) e], tr.translated (r.nextFloat() * 22 - 11, 0));
        }
        g.setColour (Colour (0xff4ff0ff).withAlpha (0.18f));
        g.fillRect (area.withHeight (6).translated (0, area.getHeight() * r.nextFloat()));
        g.setColour (Colour (0xffff3ad0).withAlpha (0.18f));
        g.fillRect (area.withHeight (4).translated (0, area.getHeight() * r.nextFloat()));
    }
    else
    {
        paintPatch (g, bodies[(size_t) e], tr);
    }

    // piezas sueltas
    if (e == 3 && ph == Phase::power) // la cabeza sigue la pastilla
    {
        const float v = window (u, 0.05f, 0.6f);
        const float follow = std::sin (MathConstants<float>::pi * v) * (u < 0.6f ? 1.0f : 0.0f);
        const float ang = -0.12f * follow + 0.05f * std::sin (MathConstants<float>::pi * window (u, 0.5f, 0.7f));
        paintPatch (g, junoHead, around (junoNeck, 1, 1, ang, { 0, -4.0f * follow }).followedBy (tr));
    }
    if (e == 5 && ph == Phase::power && u > 0.36f && u < 0.95f) // la gorra vuela
    {
        const float v = window (u, 0.36f, 0.95f);
        const float up = std::sin (MathConstants<float>::pi * v);
        const float back = v < 0.65f ? easeOutCubic (v / 0.65f) : 1 - easeInOutCubic ((v - 0.65f) / 0.35f);
        paintHole (g, dxCapArea.reduced (8, 6), Colour (0xff24160d), 0.92f * jmin (1.0f, up * 3));
        const auto cc = dxCapArea.getCentre();
        paintPatch (g, dxCap, around (cc, 1, 1, -MathConstants<float>::twoPi * 1.5f * back, { 34.0f * back, -165.0f * up }).followedBy (tr));
    }
    if (e == 7 && ph == Phase::power && u > 0.1f && u < 0.86f) // el craneo se separa, rebota y gira
    {
        const float v = window (u, 0.1f, 0.86f);
        const float h = 135.0f * (1 - v) * std::abs (std::sin (MathConstants<float>::pi * v * 2.5f));
        const float rot = MathConstants<float>::twoPi * 2.0f * easeOutCubic (v);
        paintHole (g, arpSkullArea.reduced (6), Colour (0xff0d0806), 0.95f);
        const auto sc = arpSkullArea.getCentre();
        paintPatch (g, arpSkull, around (sc, 1, 1, rot, { 26.0f * std::sin (v * 6.0f), -h }));
    }

    if (ph == Phase::hit) // destello del impacto
    {
        const auto& p = d.impact;
        g.setGradientFill (ColourGradient (Colours::white.withAlpha (0.85f * (1 - u)), p.x, p.y, Colours::transparentWhite, p.x + 46, p.y, true));
        g.fillEllipse (Rectangle<float> (92, 92).withCentre (p));
    }

    if (pu > 0.0f && pu < 1.9f)
        paintPowerFx (g, e, pu, t);
}

void AnimationLayer::paintPowerFx (Graphics& g, int e, float u, double t)
{
    const auto& d = defs[(size_t) e];
    auto drawSprite = [&] (int idx, Point<float> c, float size, float rot, float alpha)
    {
        const auto& img = sprites[(size_t) idx];
        if (! img.isValid() || alpha <= 0.0f) return;
        Graphics::ScopedSaveState s (g);
        g.setOpacity (jmin (1.0f, alpha));
        const float sc = size / (float) jmax (img.getWidth(), img.getHeight());
        g.drawImageTransformed (img, AffineTransform::translation (-img.getWidth() * 0.5f, -img.getHeight() * 0.5f)
                                         .scaled (sc).rotated (rot).translated (c.x, c.y));
    };
    auto glow = [&] (Point<float> c, float r, Colour col, float alpha)
    {
        if (alpha <= 0.0f) return;
        g.setGradientFill (ColourGradient (col.withAlpha (jmin (1.0f, alpha)), c.x, c.y, col.withAlpha (0.0f), c.x + r, c.y, true));
        g.fillEllipse (Rectangle<float> (r * 2, r * 2).withCentre (c));
    };

    switch (e)
    {
        case 0: // destellos en las mordidas
            if (u < 0.4f) glow (d.a, 30, Colour (0xffffe08a), std::abs (std::sin (u / 0.4f * MathConstants<float>::twoPi)) * 0.8f);
            break;

        case 1: // lentes que brillan + rayos astrales
        {
            const float lens = std::sin (MathConstants<float>::pi * window (u, 0.0f, 0.45f));
            for (auto p : { d.a, d.b })
            {
                glow (p, 34, Colours::white, lens);
                drawSprite (sStarB, p, 46 * lens, u * 3, lens);
            }
            const float rays = window (u, 0.28f, 0.45f) * (1 - window (u, 0.9f, 1.5f));
            if (rays > 0)
            {
                const auto c = d.body.getCentre();
                const float len = 210 * easeOutCubic (window (u, 0.28f, 0.6f));
                for (int i = 0; i < 18; ++i)
                {
                    const float ang = (float) i / 18.0f * MathConstants<float>::twoPi + (float) t * 0.0006f;
                    Path ray;
                    ray.addTriangle (c, c + Point<float> (std::cos (ang - 0.05f), std::sin (ang - 0.05f)) * len,
                                     c + Point<float> (std::cos (ang + 0.05f), std::sin (ang + 0.05f)) * len);
                    g.setGradientFill (ColourGradient ((i % 2 ? Colour (0xffffe08a) : Colour (0xff9fd8ff)).withAlpha (0.55f * rays), c.x, c.y,
                                                       Colour (0x00ffe08a), c.x + len, c.y, true));
                    g.fillPath (ray);
                }
            }
            break;
        }

        case 2: // anillos orbitales
        {
            const float k = std::sin (MathConstants<float>::pi * jlimit (0.0f, 1.0f, u));
            Graphics::ScopedSaveState s (g);
            g.addTransform (AffineTransform::rotation (-0.25f, d.a.x, d.a.y));
            for (int i = 0; i < 3; ++i)
            {
                const auto r = Rectangle<float> (236 + i * 26.0f, 80 + i * 10.0f).withCentre (d.a);
                g.setColour ((i == 1 ? Colour (0xff7fc8ff) : Colour (0xffffc040)).withAlpha (0.25f * k));
                g.drawEllipse (r, 8);
                g.setColour ((i == 1 ? Colour (0xffbfe6ff) : Colour (0xffffe08a)).withAlpha (0.85f * k));
                g.drawEllipse (r, 2);
            }
            break;
        }

        case 3: // la pastilla en parabola + arcoiris
        {
            if (u > 0.05f && u < 0.6f)
            {
                const float v = window (u, 0.05f, 0.6f);
                const Point<float> apex (368, 448);
                const auto ctrl = apex * 2.0f - (d.b + d.a) * 0.5f;
                const auto pill = quad (d.b, ctrl, d.a, v);
                glow (pill, 26, Colour (0xffff6a8a), 0.55f);
                drawSprite (sPillRed, pill, 46, v * MathConstants<float>::twoPi * 3, 1);
            }
            if (u >= 0.6f)
            {
                const float grow = easeOutBack (window (u, 0.6f, 0.82f));
                const float fade = 1 - window (u, 1.25f, 1.7f);
                drawSprite (sArcWide, { 300, 470 }, 230 * grow, -0.15f, fade);
                glow (d.a, 34, Colours::white, (1 - window (u, 0.6f, 0.75f)) * 0.9f);
            }
            break;
        }

        case 4: // ojo: carga, rayo, distorsion, flash
        {
            const float charge = window (u, 0.0f, 0.4f) * (1 - window (u, 0.75f, 1.15f));
            glow (d.a, 12 + 40 * charge, Colour (0xff9dff6a), (0.35f + 0.6f * charge) * (1 - window (u, 0.75f, 1.15f)));
            if (u > 0.4f && u < 0.78f)
            {
                const float v = window (u, 0.4f, 0.78f);
                const float w = (1 - v) * 1.0f;
                Random r ((int64) (t / 30.0));
                const Point<float> end (d.a.x + r.nextFloat() * 10 - 5, 1240);
                for (auto [width, col] : { std::pair<float, Colour> { 44 * w, Colour (0x5578ff40) },
                                           std::pair<float, Colour> { 20 * w, Colour (0xb0c8ff70) },
                                           std::pair<float, Colour> {  6 * w, Colour (0xffffffff) } })
                {
                    g.setColour (col);
                    g.drawLine ({ d.a, end }, jmax (1.0f, width + r.nextFloat() * 4));
                }
                if (u < 0.48f) // flash
                {
                    g.setColour (Colours::white.withAlpha (0.35f * (1 - window (u, 0.4f, 0.48f))));
                    g.fillRect (Rectangle<float> (190, 170, 630, 1130));
                }
            }
            const float dist = window (u, 0.4f, 1.1f);
            if (dist > 0 && dist < 1)
                for (int i = 0; i < 3; ++i)
                {
                    const float r = 30 + (dist * 160 + i * 34);
                    g.setColour (Colour (0xffb8ff7a).withAlpha (0.4f * (1 - dist)));
                    g.drawEllipse (Rectangle<float> (r * 2, r * 1.6f).withCentre (d.a), 2.0f);
                }
            break;
        }

        case 5: // brasa del porro mientras pita
            if (u < 0.36f)
            {
                const float k = 0.6f + 0.4f * std::sin ((float) t * 0.03f);
                glow (d.a.translated (8, -2), 22, Colour (0xffff7a20), k * std::sin (MathConstants<float>::pi * window (u, 0.0f, 0.36f)));
            }
            break;

        case 6: // LEDs, sobrecarga y arcos electricos
        {
            if (u < 0.45f)
            {
                const bool flip = ((int) (t / 70.0)) % 2 == 0;
                glow (d.a, 26, flip ? Colours::white : Colour (0xffff4020), 0.9f);
                glow (d.b, 26, flip ? Colour (0xffff4020) : Colours::white, 0.9f);
            }
            const float over = std::sin (MathConstants<float>::pi * window (u, 0.2f, 0.7f));
            glow (d.a.translated (20, 30), 150, Colour (0xffff9020), 0.35f * over);
            if (u > 0.45f && u < 0.85f)
            {
                Random r ((int64) (t / 45.0) + 66);
                const auto c = d.a.translated (20, 10);
                for (int i = 0; i < 4; ++i)
                {
                    const float ang = r.nextFloat() * MathConstants<float>::twoPi;
                    const auto end = c + Point<float> (std::cos (ang), std::sin (ang)) * (90 + r.nextFloat() * 90);
                    Path arc; arc.startNewSubPath (c);
                    for (int k = 1; k <= 6; ++k)
                    {
                        auto p = c + (end - c) * ((float) k / 6.0f);
                        if (k < 6) p += Point<float> (r.nextFloat() * 24 - 12, r.nextFloat() * 24 - 12);
                        arc.lineTo (p);
                    }
                    g.setColour (Colour (0x667fe7ff)); g.strokePath (arc, PathStrokeType (7));
                    g.setColour (Colours::white);      g.strokePath (arc, PathStrokeType (1.8f));
                }
            }
            break;
        }

        case 7: // brillo al volver a ensamblarse
            glow (d.b, 40, Colour (0xffffcf4a), std::sin (MathConstants<float>::pi * window (u, 0.82f, 1.0f)));
            break;

        case 8: // vortice de vinilos y humo
        {
            const float k = std::sin (MathConstants<float>::pi * jlimit (0.0f, 1.0f, u));
            if (k > 0)
                for (int i = 0; i < 4; ++i)
                {
                    const float ang = u * 7.5f + i * MathConstants<float>::halfPi;
                    const float rad = 100 - 34 * std::sin (u * MathConstants<float>::pi);
                    drawSprite (sVinyl, d.a + Point<float> (std::cos (ang) * rad, std::sin (ang) * rad * 0.75f), 74, ang * 2, k);
                }
            if (u > 0.72f) glow (d.a, 60, Colour (0xffc070ff), 1 - window (u, 0.72f, 1.2f));
            break;
        }
        default: break;
    }
}

void AnimationLayer::paintBall (Graphics& g, Point<float> p, float scale, float alpha, Point<float> vel)
{
    const auto& img = sprites[sBall];
    if (! img.isValid()) return;
    const float size = 34.0f * scale;

    // estela
    if (vel.getDistanceFromOrigin() > 2.0f)
        for (int i = 3; i >= 1; --i)
        {
            Graphics::ScopedSaveState s (g);
            g.setOpacity (alpha * 0.18f * (4 - i) / 3.0f);
            g.drawImage (img, Rectangle<float> (size, size).withCentre (p - vel * (0.6f * i)));
        }

    g.setGradientFill (ColourGradient (Colour (0xffffe6a0).withAlpha (0.35f * alpha), p.x, p.y, Colour (0x00ffe6a0), p.x + size, p.y, true));
    g.fillEllipse (Rectangle<float> (size * 2, size * 2).withCentre (p));
    Graphics::ScopedSaveState s (g);
    g.setOpacity (alpha);
    g.drawImage (img, Rectangle<float> (size, size).withCentre (p));
}

void AnimationLayer::paintPopup (Graphics& g, const Popup& pp)
{
    const float t = (float) (now - pp.start);
    const float scale = t < 220 ? easeOutBack (t / 220.0f) * 1.18f : 1.18f - 0.18f * window (t, 220, 360);
    const float alpha = 1 - window (t, (float) popupMs - 320, (float) popupMs);
    const auto c = pp.p.translated (0, -34.0f * easeOutCubic (t / (float) popupMs));

    Graphics::ScopedSaveState s (g);
    g.addTransform (AffineTransform::scale (scale, scale, c.x, c.y).rotated (-0.08f, c.x, c.y));

    // estallido rojo
    Path star; star.addStar (c, 14, 34, 54, 0.1f);
    g.setColour (Colours::black.withAlpha (0.6f * alpha));
    g.fillPath (star, AffineTransform::translation (3, 4));
    g.setGradientFill (ColourGradient (Colour (0xffff3a1a).withAlpha (alpha), c.x, c.y - 30, Colour (0xff9a0e0a).withAlpha (alpha), c.x, c.y + 40, false));
    g.fillPath (star);
    g.setColour (Colour (0xffffd25a).withAlpha (alpha));
    g.strokePath (star, PathStrokeType (2.0f));

    auto outlined = [&] (const String& text, float y, float h)
    {
        GlyphArrangement ga; ga.addLineOfText (theme::font (h), text, 0, 0);
        Path p; ga.createPath (p);
        const float w = GlyphArrangement::getStringWidth (theme::font (h), text);
        p.applyTransform (AffineTransform::translation (c.x - w * 0.5f, y + h * 0.5f));
        g.setColour (Colours::black.withAlpha (alpha));
        g.strokePath (p, PathStrokeType (h * 0.22f, PathStrokeType::curved, PathStrokeType::rounded));
        g.setGradientFill (ColourGradient (Colour (0xfffff2a0).withAlpha (alpha), c.x, y - h * 0.5f, Colour (0xffffb020).withAlpha (alpha), c.x, y + h * 0.5f, false));
        g.fillPath (p);
    };
    outlined (pp.text, c.y, 30.0f);
    if (pp.extra.isNotEmpty()) outlined (pp.extra, c.y + 52.0f, 20.0f);
}
}
