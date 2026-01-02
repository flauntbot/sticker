// SPDX-FileCopyrightText: © 2021 Chris and Flauntbot Contributors <flauntbot@chris-nz.com>
// SPDX-License-Identifier: LGPL-2.1-or-later
#ifndef STICKERGENERATOR_H
#define STICKERGENERATOR_H

class QColor;
class QPixmap;
typedef unsigned int QRgb;

#include "ChatMessage.h"
#include "Entities.h"

/*
 * Generates "stickers", which are pictures of basic, single, un-timestamped chat messages and associated user Avatars.
 * Called statically through its sole public method `generate()`.
 * The visual style is roughly modelled after how Telegram's looks, though it doesn't use a real client's algorithms.
 * The implementation is very heavily inspired by https://github.com/LyoSU/quote-api and mimics its interfaces. Roughly.
 * Under the hood, we have different tools available to us, so it ends up working a little differently and I had to
 * make a few fun choices and reimplement most of the solutions.
 * I hope I've done it justice :-)
 *
 * The upstream code does more stuff, is better tested, and produces more faithful mockups of telegram messages.
 * If you wanna see that in action, and you have telegram, check out @QuotLyBot - it's fun.
 * This is a port for a specific usecase, and here are some of the aims:
 * 1. More reliable (here). The original code ""goes to sleep"" if unused for a while, and fails its next request.
 *      I probably broke something when I installed it; I had to make a few tiny changes.
 *      I should check that out, hmmmm. Maybe the avatar caching's async functionality?
 * 2. Less memory pressure (here).
 *      My executable is 107KB right now (on disk, when stripped) and uses shared libraries already loaded.
 *      The original is using over 300MB of RAM right now, just idling.
 *      And there's a recommendation to use it with docker, on top of that.
 * 3. Less disk usage. Not a big deal, but the original needs 294MB of node_modules installed.
 *      AND they recommend using it with docker, on top of that.
 *      Mine uses whatever's lying around (if you have Qt5 in your libs path).
 *      But these days, that figure is nothing, even at home, so meh.
 * 4. Less network use
 *      The original needs a Telegram bot API key and selects the user's icon by itself.
 *      Mine assumes that you already have the user's icon, and just fetches that.
 *      I did hope that mine would run faster, but that's not proven.
 *      If used as part of a larger application (e.g. a long running server like the original) it SHOULD go faster,
 *      particularly if used behind/with a network cache.
 *
 * Theirs is designed to sit there and run at full speed 24/7 - and it does a great job.
 * Always running, never crashes, results always look great!
 * Mine is only very lightly used, at least in my case, and is intended for a much smaller purpose right now.
 *
 * If you want it battle-tested and fully featured, you do not want my stripped down implementation, you want upstream.
 */
class StickerGenerator
{
public:
    /*
     * Generates a sticker.
     *
     * @param backgroundColour - it's a numeric value like 0xffffff. Might support transparency ¯\_(ツ)_/¯
     * @param message - Contains information about the message we're drawing, including info about the sender
     * @param width - it's more of a guide, really. It may influence things like your text layout and maximum sizes
     *
     * @param scale - should adjust the relative size of some of the sticker's content, like text
     */
    static QPixmap
    generate(const QRgb &backgroundColour, ChatMessage &message, int width = 512, int scale = 2);

private:

    /*
     * Opens an HTML element in the rich text we generate for rendering
     *
     * @param type - see `Entities::styles` enum
     *
     * @return opening html tag
     */
    static QString startEntity(Styles type);

    /*
     * Closes an HTML element in the rich text we generate for rendering. Surprised?
     *
     * @param type - see `Entities::styles` enum
     *
     * @return closing html tag
     */
    static QString endEntity(Styles type);

    /*
     * Draws a substitute avatar using the user's initials
     *
     * @param user - The user
     *
     * @return picture of user's initials
     */
    static QPixmap avatarImageLetters(const ChatUser &user);

    /*
     * Uses SuperHardPoo algorithm to tell you whether your input colour is light or dark
     *
     * @param colour - the colour which is currently possibly light and/or dark
     *
     * @return true if light, false if dark
     */
    static bool isLight(const QRgb &colour);

    /*
     * Given input text and some metadata, returns a drawing of the text (with formatting)
     *
     * @param text - the text to draw
     * @param entities - formatting entities (boldness, etc) to apply to the text
     * @param fontSize - font size given in pixels (not points, because we use a perfect, imaginary display)
     * @param fontColour - colour for font ¯\_(ツ)_/¯
     * @param textX - currently unused, but is intended to give the text an offset from the left margin, in pixels
     * @param textY - rendered text's offset from top margin, in pixels
     * @param maxWidth - maximum width of rendered text, in pixels
     * @param isName - whether the text is to be drawn as a name. names require a little special treatment
     *
     * @return a picture of some text, formatted
     */
    static QPixmap drawText(QString &text,
                            const QList<Entity> &entities,
                            int fontSize,
                            const QColor *fontColour,
                            int textX,
                            int textY,
                            int maxWidth,
                            bool isName);

    /*
     * Draws a rounded rectangle
     * This implementation is a very simple wrapper around a method which, mercifully, is in the underlying library.
     *
     * @param colour - fill colour for rectangle
     * @param w - width of rectangle, in pixels
     * @param h - height of rectangle, in pixels
     * @param r - radius of corners, in pixes
     *
     * @return a drawing of a rounded rectangle
     */
    static QPixmap drawRoundRect(const QRgb &colour, int w, int h, int r);

    /*
     * Draws a vertical line next to the "replying-to section" at the top of the message.

     * Direct, untested port from Javascript/canvas code.
     *
     * @param lineWidth - width of line, in pixels
     * @param height - height, of line, in pixels
     * @param colour - colour of line
     *
     * @return picture of line
     */
    static QPixmap drawReplyLine(int lineWidth, int height, const QColor &colour);

    /*
     * Draws the user's avatar as a little circle. Will be generated from their name, if it doesn't load.
     *
     * @param user - The user who probably has an interesting name and/or avatar
     *
     * @returns a picture of the avatar
     */
    static QPixmap drawAvatar(const ChatUser &user);

    /*
     * Positions all our little pictures (including the rectangle - soon) on one big picture and returns it
     *
     * Functionality of replyName and replyText isn't yet confirmed.
     *
     * @param backgroundColour - the colour which the rounded-rectangle behind the text will be filled with
     * @param avatar - a picture of a user's avatar, either organic (from URI) or artificial (from initials)
     * @param replyName - a picture of text of an original message author's name to which this message is a reply.
     * @param replyText - a picture of text of an original message to which this message is a reply. Untested.
     * @param name - a picture of the message's author's name
     * @param text - a picture of the (formatted) text of the original message
     * @param scale - scale control for spacing and sizing of elements
     *
     * return a picture of the message, with all the details we wanted now included
     */
    static QPixmap drawQuote(QRgb backgroundColour,
                             const QPixmap &avatar,
                             const QPixmap &replyName,
                             const QPixmap &replyText,
                             const QPixmap &name,
                             const QPixmap &text,
                             int scale = 1);

};


#endif //STICKERGENERATOR_H
