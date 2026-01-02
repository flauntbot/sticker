"Sticker Generator"
===================

This is a C++ (Qt5/Qt6) partial reimplementation of https://github.com/LyoSU/quote-api -
I say "partial reimplementation" and not "port" because it does only a subset of what the quote-api
product does. quote-api is used currently for Mattata and therefore also for flauntbot/mafflebot.
quote-api is around 300MB (RAM and disk) and boasts a MASSIVE feature set. Replies, images, all sorts.

Unfortunately, mafflebot currently doesn't even have the right plumbing to send *reply* information
to quote-api. All mafflebot can do is the plain old text messages!
So quote-api is overkill for me right now. This new program occupies less than 200KB on disk (stripped) 
and gives up a lot of shiny features like avatar-caching.

Anyway, if you want a quick preview, sure there's @mafflebot but do check out Quotly on telegram 
to see the full package: https://t.me/QuotLyBot .

Here is an example (with lua) of how to call my sticker generator:
```lua
         local sender = {
            ['id'] = message.reply.forward_sender_name.length,
            ['name'] = message.reply.forward_sender_name,
            -- you could give it a path to a file like /tmp/p.jpg
            ['avatar'] = 'https://example.com/avatar'
        }
        local payload = {
            ['backgroundColor'] = '#243447',
            ['width'] = 512,
            ['scale'] = 2,
            ['message'] = {
                ['chatId'] = sender.id,
                ['from'] = sender,
                ['text'] = message.reply.text,
                --['replyMessage'] = reply,
                ['entities'] = message.reply.entities
            }
        }
        payload = json.encode(payload)
        local rr = io.popen("mtsticker ".. configuration.bot_directory .. '/output.webp', "w")
        rr:write(payload)
        rr:close()
```

and here's a IRL example for the shell:

```shell
# Your path is probably different, maybe ./sticker. Check it, lol
echo '{"backgroundColor":"#243447","width":512,"message":{"entities":[{"type":"bot_command","length":12,"offset":0}],"from":{"id":136958297,"avatar":"/tmp/photo.png","name":"Chris 🇳🇿"},"text":"/addsticker2","chatId":136958297},"scale":2}' | cmake-build-debug/sticker /tmp/beer.png
```

It needs QtGUI, which is a pretty big load. I'm lucky that I already have it in shared memory.

It's primitive enough that you shouldn't run it ""in production"" until you've audited the code, but
if you really wanna be told how to build it locally....
- my IDE does it
- You can do `mkdir build; cd build; cmake ..; make` and then run it, if you're just in a terminal

If you do check this out, I am MOST grateful for security or stability fixes, 
and don't mind if you add or suggest a feature either. But no sweat either way :-)
