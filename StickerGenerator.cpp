// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#include "StickerGenerator.h"
#include <QBitmap>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QRgb>
#include <QStaticText>
#include <QtCore>
#include <cmath>

QPixmap
StickerGenerator::generate(const QRgb &backgroundColour, ChatMessage &message, int width, int scale)
{
    // scale variable is a bit strange
    if (!scale) { scale = 2; }
    if (scale > 20) { scale = 20; }
    width *= scale;

    // we automatically have a bold entity wrapping the username, so set that up now
    QList<Entity> boldEntities;
    boldEntities.append(Entity{bold, 0, message.from.name.length()});

    // check background style colour black/light
    auto backIsLight = isLight(backgroundColour);

    // default color from tdesktop
    // https://github.com/telegramdesktop/tdesktop/blob/67d08c2d4064e04bec37454b5b32c5c6e606420a/Telegram/SourceFiles/data/data_peer.cpp#L43
    // const nameColor = [
    //   QColor(0xc0, 0x3d, 0x33),
    //   QColor(0x4f, 0xad, 0x2d),
    //   QColor(0xd0, 0x93, 0x06),
    //   QColor(0x16, 0x8a, 0xcd),
    //   QColor(0x85, 0x44, 0xd6),
    //   QColor(0xcd, 0x40, 0x73),
    //   QColor(0x29, 0x96, 0xad),
    //   QColor(0xce, 0x67, 0x1b)
    // ]

    // should find out: should i have just used QRgbs? Probably
    // name light style colour
    QList nameColorLight = {
        QColor(0x86, 0x2a, 0x23),
        QColor(0x37, 0x79, 0x1f),
        QColor(0x91, 0x66, 0x04),
        QColor(0x0f, 0x60, 0x8f),
        QColor(0x5d, 0x2f, 0x95),
        QColor(0x8f, 0x2c, 0x50),
        QColor(0x1c, 0x69, 0x79),
        QColor(0x90, 0x48, 0x12)
    };

    // name dark style color
    QList nameColorDark = {
        QColor(0xfb, 0x61, 0x69),
        QColor(0x85, 0xde, 0x85),
        QColor(0xf3, 0xbc, 0x5c),
        QColor(0x65, 0xbd, 0xf3),
        QColor(0xb4, 0x8b, 0xf2),
        QColor(0xff, 0x56, 0x94),
        QColor(0x62, 0xd4, 0xe3),
        QColor(0xfa, 0xa3, 0x57)
    };

    // user name  color
    // https://github.com/telegramdesktop/tdesktop/blob/67d08c2d4064e04bec37454b5b32c5c6e606420a/Telegram/SourceFiles/data/data_peer.cpp#L43
    auto nameMap = QList{0, 7, 4, 1, 6, 3, 5};

    auto nameIndex = std::fmod(qAbs(message.from.id), 7);

    auto nameColorIndex = nameMap.at(static_cast<int>(nameIndex));
    auto nameColorPalette = backIsLight ? nameColorLight : nameColorDark;
    auto nameColor = nameColorPalette[nameColorIndex];

    auto nameSize = 22 * scale;

    // where we write the peer's/user's name (if there is one)
    QPixmap nameCanvas;
    if (!message.from.name.isEmpty()) {
        nameCanvas = drawText(message.from.name,
                              boldEntities,
                              nameSize,
                              &nameColor,
                              0,
                              0,
                              width, true);
    }

    // const minFontSize = 18
    // const maxFontSize = 28

    // let fontSize = 25 / ((text.length / 10) * 0.2)

    // if (fontSize < minFontSize) fontSize = minFontSize
    // if (fontSize > maxFontSize) fontSize = maxFontSize

    auto fontSize = 24 * scale;

    auto textColor = backIsLight ? QColor(0, 0, 0) : QColor(0xff, 0xff, 0xff);

    // The message body. Only text is supported, but with lots of formatting
    QPixmap textCanvas;
    if (!message.text.isEmpty()) {
        textCanvas = drawText(message.text,
                              message.entities,
                              fontSize,
                              &textColor,
                              0,
                              0,
                              width, false);
    }

    // This becomes the user's avatar, cropped to a circle. OR, their initials, on a circular background
    QPixmap avatarCanvas = drawAvatar(message.from);

    // This is completely untested and probably won't work at all, but the meat is here
    QPixmap replyName;
    QPixmap replyText;
    if (message.replyMessage && !message.replyMessage->from.name.isEmpty() && !message.replyMessage->text.isEmpty()) {
        auto replyNameIndex = fmod(qAbs(message.replyMessage->from.id), 7);
        // narrowing!
        auto replyNameColorIndex = nameMap.at(static_cast<int>(replyNameIndex));
        auto colorSet = backIsLight ? nameColorLight : nameColorDark;
        auto replyNameColor = colorSet[replyNameColorIndex];

        auto replyNameFontSize = 16 * scale;

        if (!message.replyMessage->from.name.isEmpty()) {
            replyName = drawText(message.replyMessage->from.name,
                                 boldEntities,
                                 replyNameFontSize,
                                 &replyNameColor,
                                 0,
                                 replyNameFontSize,
                                 static_cast<int>(width * 0.9), true);
        }

        auto textColor2 = backIsLight ? QColor(0, 0, 0) : QColor(0xff, 0xff, 0xff);

        auto replyTextFontSize = 21 * scale;
        // FIXME rounded double to int, but double might be wiser anyway
        replyText = drawText(message.replyMessage->text,
                             QList<Entity>(),
                             replyTextFontSize,
                             &textColor2,
                             0,
                             replyTextFontSize,
                             qRound(width * 0.9), false);
    }

    // so now send all the little pictures to drawQuote for compositing or what have you
    return drawQuote(
        backgroundColour,
        avatarCanvas,
        replyName, replyText,
        nameCanvas, textCanvas,
        scale
    );

}

bool StickerGenerator::isLight(const QRgb &colour)
{
    // https://codepen.io/andreaswik/pen/YjJqpK

    // HSP (Highly Sensitive Poo) equation from http://alienryderflex.com/hsp.html
    const auto hsp = qSqrt(
        0.299 * (qRed(colour) * qRed(colour)) +
            0.587 * (qGreen(colour) * qGreen(colour)) +
            0.114 * (qBlue(colour) * qBlue(colour))
    );

    // Using the HSP value, determine whether the colour is light or dark
    return hsp > 127.5;
}

QPixmap StickerGenerator::drawQuote(const QRgb backgroundColour,
                                    const QPixmap &avatar,
                                    const QPixmap &replyName,
                                    const QPixmap &replyText,
                                    const QPixmap &name,
                                    const QPixmap &text,
                                    const int scale)
{
    // for the rectangle/bubble behind the name and the text body
    const auto blockPosX = 55 * scale;
    constexpr auto blockPosY = 0;

    // general measurement for padding, minimum sizes, and replied-to indents
    const auto indent = 15 * scale;

    const auto avatarSize = 50 * scale;

    // calculate the width for our background box/bubble by finding which picture will need the most horizontal space
    auto width = 0;
    if (!name.isNull()) { width = name.width(); }
    if (!text.isNull() && width < text.width()) { width = text.width() + indent; }
    if (!replyName.isNull()) {
        if (width < replyName.width()) { width = replyName.width() + indent; }
        if (width < replyText.width()) { width = replyText.width() + indent; }
    }

    // we don't need height if we have nothing
    double height = 0;
    if (name.isNull() && !text.isNull()) {
        // height when we text but no name
            height = text.height() + indent;
    } else if (!name.isNull() && !text.isNull()) {
        // height if we have both name and text
            height = text.height() + name.height();
    } else if (!name.isNull() && text.isNull()) {
        // height if we have name only
            height = 2 * indent;
    }

    width += blockPosX + indent * 2;
    height += blockPosY;

    int namePosX;
    int namePosY;

    if (name.isNull()) {
        namePosX = 0;
        namePosY = -indent - 5 * scale;
    } else {
        namePosX = blockPosX + indent;
        namePosY = indent;
    }

    const auto textPosX = blockPosX + indent;
    double textPosY = indent;
    if (!name.isNull()) { textPosY = name.height() - 5 * scale; }

    auto replyPosX = 0;
    double replyNamePosY = 0;
    double replyTextPosY = 0;

    // remember that reply isn't actually tested (I was drawing from a system that can't support replies)
    if (!replyName.isNull()) {
        replyPosX = textPosX + indent;

        const auto replyNameHeight = replyName.height() * 1.2;
        const auto replyTextHeight = replyText.height() * 0.5;

        replyNamePosY = namePosY + replyNameHeight;
        replyTextPosY = replyNamePosY + replyTextHeight;

        textPosY += replyNameHeight + replyTextHeight;
        height += replyNameHeight + replyTextHeight;
    }

    height -= 11 * scale;

    QPixmap canvas(width, static_cast<int>(height));
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    const auto rectWidth = width - blockPosX;
    const auto rectHeight = height;
    const auto rectPosX = blockPosX;
    constexpr auto rectPosY = blockPosY;
    const auto rectRoundRadius = 25 * scale;

//    finally we draw the box/rectabngle/bubble behind the name and the message text
    QPixmap rect;
    if (!name.isNull() || !replyName.isNull()) {
        rect = drawRoundRect(backgroundColour,
                             rectWidth,
                             static_cast<int>(rectHeight),
                             rectRoundRadius);
    }

    // avatar at top, just left of text box
    if (!avatar.isNull()) {
        constexpr auto avatarPosY = 15;
        constexpr auto avatarPosX = 0;
        painter.drawPixmap(avatarPosX, avatarPosY,
                           avatar.scaled(avatarSize, avatarSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    // text box big enough to hold name and text
    if (!rect.isNull()) { painter.drawPixmap(rectPosX, rectPosY, rect); }
    // name is at top of text box
    if (!name.isNull()) { painter.drawPixmap(namePosX, namePosY - scale, name); }
    // text is in text box under name
    if (!text.isNull()) { painter.drawPixmap(textPosX, static_cast<int>(textPosY), text); }

    // if we have a reply (please no), we can adjust things a bit. Not tested.
    if (!replyName.isNull()) {
        const auto lineColor = isLight(backgroundColour) ? QColor(0, 0, 0) : QColor(0xff, 0xff, 0xff);
        painter.drawPixmap(textPosX,
                           static_cast<int>(replyNamePosY),
                           drawReplyLine(3 * scale, static_cast<int>(replyName.height() + replyText.height() * 0.4), lineColor));

        painter.drawPixmap(replyPosX, static_cast<int>(replyNamePosY), replyName);
        painter.drawPixmap(replyPosX, static_cast<int>(replyTextPosY), replyText);
    }

    // we just return the QPixmap to the calling function. They can decide what to do with it.
    return canvas;
}

QPixmap StickerGenerator::drawText(QString &text,
                                   const QList<Entity> &entities,
                                   const int fontSize,
                                   const QColor *fontColour,
                                   int textX,
                                   const int textY,
                                   int maxWidth,
                                   const bool isName)
{
    if (maxWidth > 10000) { maxWidth = 10000; }
//    if (maxHeight > 10000) maxHeight = 10000;
//    auto lineHeight = 4 * (fontSize * 0.3);

    constexpr auto lineHeight = 90;
    // future: support more fonts (specified in input json?) and other emoji typefaces
    /*
     * Desktop TG actually uses OpenSans. Quote-API project uses NotoSans (and San Francisco Mono).
     * Android and IOS both probably use default platform font set by user.
     * OpenSans as installed on my system doesn't do half the things I'd like
     * (or support all the characters I encounter), so the Noto family is a GREAT choice.
     * Monospace font is set by inline styles in the rich text, and is Noto Serif Mono I think.
     * So, there's a little bit of freedom to choose with font faces.
     *
     * In the future I'd like to support some styling (ESPECIALLY font faces) in the configuration/input.
     * Specifically, I would like to render tc in something like chococooky.
     */
    const QString fontName("NotoSans");
//    QString emojiFontName = QFD::applicationFontFamilies(QFD::addApplicationFont("/tmp/AppleColorEmoji.ttf"))[0];

// telegram convention, really. Could it be configurable? Because for a wee graphic, this might be HUUUUGE
    text.truncate(4096);

// We copy the input text from the original so that I can still use the original for measurements and estimations
    QString str(text);

    // commented out because it's not needed in this deployment (but it likely will be in another)
//    QRegExp rx("(\\\\u[0-9a-fA-F]{4})");
//    int pos = 0;
//    while ((pos = rx.indexIn(str, pos)) != -1) {
//        str.replace(pos++, 6, QChar(rx.cap(1).right(4).toUShort(nullptr, 16)));
//    }

    // making these literal lets us do things like process it as ONE character (see below for-loop)
    str.replace(R"(\n)", "\n");

    struct EntitySpan {
        Styles type;
        int start;
        int end;
    };

    QVector<EntitySpan> spans;
    spans.reserve(entities.size());
    const auto textLen = str.length();
    for (const auto &[type, offset, length] : entities) {
        if (length <= 0) {
            continue;
        }
        auto start = offset;
        auto end = start + static_cast<int>(length);
        if (end <= 0 || start >= textLen) {
            continue;
        }
        if (start < 0) {
            start = 0;
        }
        if (end > textLen) {
            end = textLen;
        }
        spans.push_back({type, start, end});
    }

    auto spanIndex = 0;
    QVector<EntitySpan> stack;
    stack.reserve(spans.size());

    // This little HTML preamble means we don't need to use textlayout class, and everything is automatic
    QString processed("<div style='line-height: %1%;'>");
    processed = processed.arg(lineHeight);

    // we iterate over our entities and use them to start and end HTML tags for rich text formatting.
    // this is only intended for use where entities can nest (<b><i></i></b>) but NOT otherwise overlap.
    // if your entities overlap (like <b><i></b></i>) it may or may not still work as intended
    for (auto i = 0; i < str.length(); ++i) {
        // this seems like it'd be spectacularly inefficient. I hope qt's doing some magic underneath
        while (spanIndex < spans.size() && spans[spanIndex].start == i) {
            processed.append(startEntity(spans[spanIndex].type));
            stack.push_back(spans[spanIndex]);
            ++spanIndex;
        }

        // now escape the character itself as HTML (because we are going to need the actual html)
        processed.append(QString(str[i]).toHtmlEscaped());

        while (!stack.isEmpty() && stack.last().end == i + 1) {
            processed.append(endEntity(stack.last().type));
            stack.removeLast();
        }

        /*
         * and if the character was a newline, insert it (after opening and closing entities, because inline elements)
         * as a HTML4ish line break element
         */
        if (str[i] == '\n') {
            processed.append("<br>");
        }

    }

    while (!stack.isEmpty()) {
        processed.append(endEntity(stack.last().type));
        stack.removeLast();
    }

    // unescaping something. needed in a future implementation, but not this one right now
//    processed = processed.replace(R"(\/)", "/");
//    processed = processed.replace(R"(\\)", "\\") + "</div>";

// close what we opened in the "preamble"
    processed += "</div>";

    /*
     * On ANDROID, telegram is allowed to use system fonts, and so it can represent all kinds of weird scripts
     * with characters wayyyyy off the BMP. I talk to a guy whose name is written in Old Turkic.
     * Android tg can draw it. Desktop tg on my system cannot.
     * Loading all these fallback fonts allows us to do the cool thing that the Android version does.
     * We need to include the emoji font, because (duh) otherwise we don't get many emoji (the combined ones!).
     * If you want to switch out the emoji font, you MIGHT need to load it as the main font and then use the main font
     * as a fallback font. Unconfirmed.
     * android, ios and desktop all use apple emoji, usually, so using Noto is a visible deviation!
     * The others are all consistent!
     * However, for MOST of the VERY FUNDAMENTAL emoji, the similarity is usually high enough for it to be OK.
     * And sometimes packagers change the fonts, lol, so I THINK the debian version uses noto emoji anyway.
     */
    const auto fonts = QString(
        "NotoColorEmoji "
        "NotoSansAdlam NotoSansAdlamUnjoined NotoSansAnatolianHieroglyphs NotoSansArabic "
        "NotoSansArmenian NotoSansAvestan NotoSansBalinese NotoSansBamum NotoSansBassaVah "
        "NotoSansBatak NotoSansBengali NotoSansBhaiksuki NotoSansBrahmi NotoSansBuginese "
        "NotoSansBuhid NotoSansCanadianAboriginal NotoSansCarian NotoSansCaucasianAlbanian "
        "NotoSansChakma NotoSansCham NotoSansCherokee NotoSansCoptic NotoSansCuneiform "
        "NotoSansCypriot NotoSansDeseret NotoSansDevanagari NotoSansDisplay NotoSansDuployan "
        "NotoSansEgyptianHieroglyphs NotoSansElbasan NotoSansElymaic NotoSansEthiopic "
        "NotoSansGeorgian NotoSansGlagolitic NotoSansGothic NotoSansGrantha NotoSansGujarati "
        "NotoSansGunjalaGondi NotoSansGurmukhi NotoSansHanifiRohingya NotoSansHanunoo "
        "NotoSansHatran NotoSansHebrew NotoSansImperialAramaic NotoSansIndicSiyaqNumbers "
        "NotoSansInscriptionalPahlavi NotoSansInscriptionalParthian NotoSansJavanese "
        "NotoSansKaithi NotoSansKannada NotoSansKayahLi NotoSansKharoshthi NotoSansKhmer "
        "NotoSansKhojki NotoSansKhudawadi NotoSansLao NotoSansLepcha NotoSansLimbu "
        "NotoSansLinearA NotoSansLinearB NotoSansLisu NotoSansLycian NotoSansLydian "
        "NotoSansMahajani NotoSansMalayalam NotoSansMandaic NotoSansManichaean NotoSansMarchen "
        "NotoSansMasaramGondi NotoSansMath NotoSansMayanNumerals NotoSansMedefaidrin "
        "NotoSansMeeteiMayek NotoSansMendeKikakui NotoSansMeroitic NotoSansMiao NotoSansModi "
        "NotoSansMongolian NotoSansMro NotoSansMultani NotoSansMyanmar NotoSansNKo "
        "NotoSansNabataean NotoSansNewTaiLue NotoSansNewa NotoSansNushu NotoSansOgham "
        "NotoSansOlChiki NotoSansOldHungarian NotoSansOldItalic NotoSansOldNorthArabian "
        "NotoSansOldPermic NotoSansOldPersian NotoSansOldSogdian NotoSansOldSouthArabian "
        "NotoSansOldTurkic NotoSansOriya NotoSansOsage NotoSansOsmanya NotoSansPahawhHmong "
        "NotoSansPalmyrene NotoSansPauCinHau NotoSansPhagsPa NotoSansPhoenician "
        "NotoSansPsalterPahlavi NotoSansRejang NotoSansRunic NotoSansSamaritan NotoSansSaurashtra "
        "NotoSansSharada NotoSansShavian NotoSansSiddham NotoSansSignWriting NotoSansSinhala "
        "NotoSansSogdian NotoSansSoraSompeng NotoSansSoyombo NotoSansSundanese "
        "NotoSansSylotiNagri NotoSansSymbols NotoSansSymbols2 NotoSansSyriac NotoSansTagalog "
        "NotoSansTagbanwa NotoSansTaiLe NotoSansTaiTham NotoSansTaiViet NotoSansTakri "
        "NotoSansTamil NotoSansTamilSupplement NotoSansTelugu NotoSansThaana NotoSansThai "
        "NotoSansTifinagh NotoSansTifinaghAPT NotoSansTifinaghAdrar "
        "NotoSansTifinaghAgrawImazighen NotoSansTifinaghAhaggar NotoSansTifinaghAir "
        "NotoSansTifinaghAzawagh NotoSansTifinaghGhat NotoSansTifinaghHawad "
        "NotoSansTifinaghRhissaIxa NotoSansTifinaghSIL NotoSansTifinaghTawellemmet "
        "NotoSansTirhuta NotoSansUgaritic NotoSansVai NotoSansWancho NotoSansWarangCiti "
        "NotoSansYi NotoSansZanabazarSquare").split(" ");

    QFont::insertSubstitutions(fontName, fonts);

    QFont font(fontName);
//    QFont font = QFont("ChocoCooky");
    font.setPixelSize(fontSize);
    font.setHintingPreference(QFont::PreferNoHinting);
    font.setStyleStrategy(QFont::PreferOutline);
    font.setStyleHint(QFont::SansSerif);

    // for measuring text's needed width/height space using the original input
    const QFontMetrics fm(font);

    // If you haven't seen this, it's a pre-rendered QString and it accepts rich-text (which in Qt is basically HTML)
    // QStaticText can do word wrapping and stuff automatically.
    QStaticText staticText(processed);
    staticText.setTextFormat(Qt::RichText);
    staticText.prepare(QTransform(), font);

    if (isName && staticText.size().width() > maxWidth) {
        // can't draw text past the edge of the box. This is not great protection. rare case, too
        staticText.setTextWidth(maxWidth);
    } else if (!isName) {
        // If it's not the name, then it's message text or avatar text (for now).
        // Desktop seems to trim to 45 or 50 characters roughly of OpenSans, so I emulate that.
        // if the text height is more than 1 (and a half) lines of how tall the font is, give it more horizontal space.
        if (const auto max_text_width = fm.horizontalAdvance(text.left(45)); staticText.size().width() < max_text_width && staticText.size().height() > fm.height() * 1.5) {
            staticText.setTextWidth(max_text_width + 1);
        }
    } else {
        // if it's a name and it isn't longer than the max width...
        QTextOption alignment;
        // FORCE it to the left of the space, like in android and desktop, even if it's RTL
        alignment.setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
        staticText.setTextOption(alignment);
        // Allow it a little extra horizontal space to prevent accidental wrapping
        staticText.setTextWidth(fm.size(Qt::TextSingleLine, text).width() * 1.5);
    }

    // Now we are gonna adjust our paint area to fit our text, and then we're gonna actually draw the text
    const auto sz = staticText.size();
    QPixmap canvas(static_cast<int>(sz.width()), static_cast<int>(sz.height()) + fontSize);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    painter.setFont(font);
    painter.setPen(*fontColour);

    painter.drawStaticText(textX, textY, staticText);

    // so there you go, a picture of text
    return canvas;
}

QPixmap StickerGenerator::drawRoundRect(const QRgb &colour, const int w, const int h, int r)
{
    // The painter doesn't have a roundrect method, but the path tool does. So let's get set up....

    if (w < 2 * r) { r = w / 2; }
    if (h < 2 * r) { r = h / 2; }

    auto im = QPixmap(w, h);
    im.fill(Qt::transparent);
    QPainter painter(&im);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(colour);

    // now we create our rounded rect as a PATH
    QPainterPath path;
    path.addRoundedRect(0, 0, w, h, r, r);

    // and boingo, draw the path, job done
    painter.fillPath(path, QColor(colour));
    painter.drawPath(path);

    return im;
}

QPixmap StickerGenerator::drawReplyLine(const int lineWidth, const int height, const QColor &colour)
{
    // not tested. here be dragons.
    QPixmap canvas(20, height);
    QPainter painter(&canvas);
    auto pen = painter.pen();
    pen.setColor(colour);
    pen.setWidth(lineWidth);
    painter.drawLine(QPointF(10, 0), QPointF(10, height));
    return canvas;
}

QPixmap StickerGenerator::drawAvatar(const ChatUser &user)
{
    QPixmap avatarImage;

    // This will load from any URI it likes - file, http etc.
    // Caching is not really my job. Use a local picture or set up a QNetworkManager.
    avatarImage.load(user.avatar);

    // generate a picture using user's initials, if we failed to load one from input
    if (avatarImage.isNull()) {
        avatarImage = avatarImageLetters(user);
    }

    // just get the image dimensions and draw a rounded rect mask over it with such big corners that it becomes a circle
    const auto w = avatarImage.width();
    const auto h = avatarImage.height();
    auto mask = QBitmap(w, h);
    mask.fill(Qt::color0);
    QPainter painter2(&mask);
    painter2.setBrush(Qt::color1);
    // yeah, right
    const auto r = w < h ? w / 2 : h / 2;
    painter2.drawRoundedRect(0, 0, w, h, r, r);
    avatarImage.setMask(mask);

    return avatarImage;
}

QPixmap StickerGenerator::avatarImageLetters(const ChatUser &user)
{
    // this is harder than it looks because of WIDE characters like emoji

    // inconsistency: "]" is a word boundary. I've split on that. is this wrong? other impls just pick space
    // I could use https://doc.qt.io/qt-5/qchar.html#isSpace-1 or something
    // You'll notice that the different mainstream clients don't perfectly match on drawing this. I took some liberties.
    QString letters;
    if (!user.first_name.isEmpty() && !user.last_name.isEmpty()) {
        auto fnFinder = QTextBoundaryFinder(QTextBoundaryFinder::Grapheme, user.first_name);
        auto lnFinder = QTextBoundaryFinder(QTextBoundaryFinder::Grapheme, user.last_name);
        letters = user.first_name.left(fnFinder.toNextBoundary()) + user.last_name.left(lnFinder.toNextBoundary());
    } else {
        QString name = user.first_name.isEmpty() ? user.name : user.first_name;
        name = name.toUpper();
        auto nFinder = QTextBoundaryFinder(QTextBoundaryFinder::Word, name);
        // probably don't need to find first word boundary. if you think about it
        QString fw = name.left(nFinder.toNextBoundary());
        auto fnFinder = QTextBoundaryFinder(QTextBoundaryFinder::Grapheme, fw);
        QString fn = fw.left(fnFinder.toNextBoundary());
        // I should probably test for space
        nFinder.toEnd();
        auto index = nFinder.toPreviousBoundary();
        QString lw = index ? name.right(name.length() - index) : "";
        auto lnFinder = QTextBoundaryFinder(QTextBoundaryFinder::Grapheme, lw);
        QString ln = lw.left(lnFinder.toNextBoundary());

        letters = fn + ln;
    }

    // so like before... this could be an array of QRgbs (ints)
    QList avatarColorArray = {
        QColor(0xc0, 0x3d, 0x33),
        QColor(0x4f, 0xad, 0x2d),
        QColor(0xd0, 0x93, 0x06),
        QColor(0x16, 0x8a, 0xcd),
        QColor(0x85, 0x44, 0xd6),
        QColor(0xcd, 0x40, 0x73),
        QColor(0x29, 0x96, 0xad),
        QColor(0xce, 0x67, 0x1b)
    };

    auto colorMapId = QList{0, 7, 4, 1, 6, 3, 5};
    auto nameIndex = static_cast<int>(std::fmod(qAbs(user.id), 7));

    auto color = avatarColorArray[colorMapId[nameIndex]];

    auto size = 500;
    auto canvas = QPixmap(size, size);
    QPainter painter(&canvas);

    auto white = QColor(0xffffff << 0);

    // just draw a big square over the whole thing, to fill it
    painter.fillRect(0, 0, canvas.width(), canvas.height(), color);

    // ask for our text (1 or 2 letters chosen above) to be drawn on a wee picture
    auto drawLetters = drawText(letters, QList<Entity>(), static_cast<int>(size / 2.5),
                                &white, 0, 0, static_cast<int>(size / 1.1), false);

    // fit the picture onto the background thing.
    // I could use the gravity/centre thing if this isn't sufficiently accurate.
    painter.drawPixmap((canvas.width() - drawLetters.width()) / 2,
                       (canvas.height() - drawLetters.height()) * 2,
                       drawLetters);

    return canvas;
}

QString StickerGenerator::startEntity(const Styles type)
{
    // inconsistency: font styles (esp families) and colours
     /*
     * TDesktop can use monospace, Consolas, Liberation Mono, Menlo, Courier I think?
     * the monospace font looks subtly different to the opensans font, idk.
     * On android the monospace font is with serifs. On MY tdekstop, the font difference is not exactly striking.
     */
     /*
      * the colours are copied from default platform themes, Quote-API, etc. But I don't like them :-(
      * Hyperlinks don't jump out enough, and monospace text maybe doesn't need overloaded with visual cues.
      * I do let the monospace text be a different colour in clients I use, but I think it looks better in these
      * pictures to NOT do that.
      */
    switch (type) {
        case bold:
            return "<b>";
        case bot_command:
        case cashtag:
        case email:
        case hashtag:
        case mention:
        case text_link:
        case url:
            // we do not care where the URL goes
            return "<a href='about:blank' style='/*color: #6ab7ec;*/'>";
        case
            code:
                // do i need to include a longer list of font families here, or can we treat Noto Mono as a hard req?
                // I guess if it's missing they'll still get the weight difference at least.
            return "<code style='font-weight: 100; font-family: \"Noto Mono\", Courier, monospace, ui-monospace; /*color: #5887a7;*/'>";
        case pre:
            return "<pre style='font-weight: 100; font-family: \"Noto Mono\", Courier, monospace, ui-monospace; /*color: #5887a7;*/'>";
        case italic:
            return "<i>";
        case strikethrough:
            return "<s>";
        case underline:
            return "<u>";
        case phonenumber:
            // phone number entities show up at wrong times, and i don't see them on desktop, so i decided to ignore.
        default:
            return "";
    }
}

QString StickerGenerator::endEntity(const Styles type)
{
    switch (type) {
        case bold:
            return "</b>";
        case bot_command:
        case cashtag:
        case email:
        case hashtag:
        case mention:
        case text_link:
        case url:
            return "</a>";
        case
            code:
            return "</code>";
        case pre:
            return "</pre>";
        case italic:
            return "</i>";
        case strikethrough:
            return "</s>";
        case underline:
            return "</u>";
        case phonenumber:
        default:
            return "";
    }
}
