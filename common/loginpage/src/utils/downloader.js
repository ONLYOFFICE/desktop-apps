'use strict';

const URL_PROXY_SCHEME = 'onlyoffice-proxy://';

export class Downloader {
  constructor({ url, size, onProgress, onComplete, onError }) {
    this.url = url;
    // this.fileName = fileName;
    this.expectedTotal = size || 0;
    this.onProgress = onProgress || (() => {});
    this.onComplete = onComplete || (() => {});
    this.onError = onError || (() => {});
    this.controller = null;
  }

  // to cancel download
  abort() {
    if (this.controller) this.controller.abort();
  }

  async start() {
    if (!navigator.onLine) {
      const err = new Error('offline! check internet connection');
      err.code = 'OFFLINE';
      this.onError(err);
      throw err;
    }

    // to catch interrupted connection
    this.controller = new AbortController();
    const handleOffline = () => this.controller.abort();
    window.addEventListener('offline', handleOffline);

    try {
      let response;
      try {
        response = await fetch(URL_PROXY_SCHEME + this.url, { signal: this.controller.signal });
      } catch (err) {
        throw this._toFriendlyError(err, false);
      }

      if (!response.ok) {
        const err = new Error(`server error ${response.status} ${response.statusText}`);
        err.code = 'HTTP_ERROR';
        err.status = response.status;
        throw err;
      }

      // when browser does not support read to stream
      // if (!response.body) {}

      const headerContentLength = response.headers.get('Content-Length');
      const headerTotal = headerContentLength ? parseInt(headerContentLength, 10) : 0;
      const total = headerTotal || this.expectedTotal;
      let loaded = 0, tmppath;

      const reader = response.body.getReader();

      let done, value;
      for (;;) {
        try {
          ({ done, value } = await reader.read());
        } catch (err) {
          throw this._toFriendlyError(err, true);
        }
        if (done) break;

        tmppath = window.AscDesktopEditor.writeTmpFile(value);

        loaded += value.length;
        this.onProgress({
          loaded,
          total,
          percent: total ? Math.min(100, Math.round((loaded / total) * 100)) : null,
        });
      }

      // check file integrity by comparing the length from request header and received size
      if (headerTotal > 0 && loaded !== total) {
        const err = new Error(`Загрузка прервана: получено ${loaded} из ${total} байт`);
        err.code = 'INCOMPLETE';
        throw err;
      }

      this.onComplete(tmppath);
      return tmppath;
    } catch (err) {
      this.onError(err);
      throw err;
    } finally {
      window.removeEventListener('offline', handleOffline);
      this.controller = null;
    }
  }

  _toFriendlyError(err, duringDownload) {
    if (err.name === 'AbortError') {
      const wrapped = new Error(
        !navigator.onLine ? 'The connection was interrupted during download' : 'Download canceled'
      );
      wrapped.code = 'ABORTED';
      wrapped.original = err;
      return wrapped;
    }
    if (err instanceof TypeError) {
      const wrapped = new Error(
        duringDownload ?
          'The connection was interrupted during download. Check your internet connection and try again.' :
          'Failed to connect to the server. Check your internet connection'
      );
      wrapped.code = 'NETWORK_ERROR';
      wrapped.original = err;
      return wrapped;
    }
    return err;
  }
}
