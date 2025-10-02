#!/usr/bin/env python3
import sys
import os
import json
import yt_dlp

def progress_hook(d: dict):
    try:
        status = d.get('status')

        if status == 'downloading':
            filename = d.get('filename')
            if filename:
                root, ext = os.path.splitext(filename)
                if ext == ".webp":
                    return

            info = d.get('info_dict') or {}
            playlist_index = info.get('playlist_index', 1) or 1
            playlist_count = info.get('n_entries', 1) or 1

            percent = None
            try:
                if d.get('total_bytes'):
                    percent = d['downloaded_bytes'] * 100.0 / d['total_bytes']
                elif d.get('total_bytes_estimate'):
                    percent = d['downloaded_bytes'] * 100.0 / d['total_bytes_estimate']
                elif 'percent' in d:
                    percent = float(d['percent'])
            except Exception:
                percent = None

            pct_str = f"{percent:.3f}" if percent is not None else "NA"
            print(f"DOWN {playlist_index}/{playlist_count} {pct_str}", flush=True)

        elif status == 'finished':
            info = d.get('info_dict') or {}
            playlist_index = info.get('playlist_index', 1) or 1
            playlist_count = info.get('n_entries', 1) or 1
            print(f"FINI {playlist_index}/{playlist_count}", flush=True)

        elif status == 'error':
            error_message = d.get('error', 'Unknown error')
            info = d.get('info_dict') or {}
            playlist_index = info.get('playlist_index', '?')
            playlist_count = info.get('n_entries', '?')
            print(f"ERRO {playlist_index}/{playlist_count} {error_message}", flush=True)

        else:
            # For unexpected statuses
            print(f"STAT {status}: {json.dumps(d, default=str)}", flush=True)

    except Exception as e:
        # Fallback: log the exception safely
        print(f"EXCE: {type(e).__name__}: {e}", flush=True)

def main():
    optionsFileName = sys.argv[1]

    if not optionsFileName:
        print('No options json file provided.', file=sys.stderr)

    with open(optionsFileName, 'r') as optionsfile:
        options = dict(json.load(optionsfile))

    urls = options['urls']
    if not urls:
        print('No urls provided', file=sys.stderr)

    options.pop('urls')

    ydl_opts_from_flags = options

    # Insert our progress hook
    hooks = ydl_opts_from_flags.get('progress_hooks', [])
    # convert to list if not
    if not isinstance(hooks, list):
        hooks = [hooks]
    hooks.append(progress_hook)
    ydl_opts_from_flags['progress_hooks'] = hooks

    ydl_opts_from_flags['quiet'] = True
    ydl_opts_from_flags['noprogress'] = True
    ydl_opts_from_flags['no_warnings'] = True

    # Now run download
    with yt_dlp.YoutubeDL(ydl_opts_from_flags) as ydl:
        ydl.download(urls)

if __name__ == '__main__':
    main()
