#include <stdlib.h>
#include <semaphore.h>

#include "csapp.h"
#include "pbx.h"
#include "debug.h"

struct pbx {
    TU** registry;
    sem_t mutex;
};

struct tu {
    int ext;
    int fd;
    TU_STATE state;
    TU* connection;
};



/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
PBX *pbx_init() {
    pbx = malloc(sizeof(struct pbx));
    if(pbx==NULL) {
        debug("malloc error");
        return NULL;
    }
    pbx->registry = malloc(FD_SETSIZE*sizeof(struct tu*)); // Maximum number of extensions supported by a PBX.
    if(pbx->registry==NULL) {
        debug("malloc error");
        free(pbx);
        return NULL;
    }
    for(int i=0; i<FD_SETSIZE; i++) {
        pbx->registry[i] = NULL;
    }
    Sem_init(&(pbx->mutex), 0, 1);
    return pbx;
}

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */
void pbx_shutdown(PBX *pbx) {

}

/*
 * Register a TU client with a PBX.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is assigned an extension number and it is initialized to the TU_ON_HOOK state.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX.
 * @param fd  File descriptor providing access to the underlying network client.
 * @return A TU object representing the client TU, if registration succeeds, otherwise NULL.
 * The caller is responsible for eventually calling pbx_unregister to free the TU object
 * that was returned.
 */
TU *pbx_register(PBX *pbx, int fd) {
    TU* tu = malloc(sizeof(TU));
    tu->fd = fd;
    tu->state = TU_ON_HOOK;

    TU** clients = pbx->registry;
    P( &(pbx->mutex) );
    for(int i=0; i<FD_SETSIZE; i++) {
        if( clients[i] == NULL ){
            tu->ext = i;
            clients[i] = tu;
            break;
        }
    }
    V( &(pbx->mutex) );
    return tu;
}

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * This object is freed as a result of the call and must not be used again.
 * @return 0 if unregistration succeeds, otherwise -1.
 */
int pbx_unregister(PBX *pbx, TU *tu) {
    int i = tu->ext;
    if( pbx->registry[i]==NULL ) { // if the TU is not in the registry
        return -1;
    } else {
        P( &(pbx->mutex) );
        pbx->registry[i] = NULL;
        free(tu);
        V( &(pbx->mutex) );
        return 0;
    }
}

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 */
int tu_fileno(TU *tu) {
    return tu->fd;
}

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 *
 * @param tu
 * @return the extension number, if any, otherwise -1.
 */
int tu_extension(TU *tu) {
    return tu->ext;
}

/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *
 *   If the TU was in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *   If the TU was in any other state, then it remains in that state.
 *
 * In all cases, a notification of the new state is sent to the network client
 * underlying this TU. In addition, if the new state is TU_CONNECTED, then the
 * calling TU is also notified of its new state.
 *
 * @param tu  The TU that is to be taken off-hook.
 * @return 0 if successful, -1 if any error occurs.  Note that "error" refers to
 * an underlying I/O or other implementation error; a transition to the TU_ERROR
 * state (with no underlying implementation error) is considered a normal occurrence
 * and would result in 0 being returned.
 */
int tu_pickup(TU *tu) {
    int fd = tu->fd;
    FILE* write = fdopen(fd, "w");
    if(tu->state==TU_ON_HOOK) {
        tu->state = TU_DIAL_TONE;
    } else if(tu->state==TU_RINGING) {
        tu->state = TU_CONNECTED;
        TU* connection = tu->connection;
        connection->state = TU_CONNECTED;

        // the calling TU is also notified of its new state
        fputs(tu_state_names[TU_CONNECTED], fdopen(connection->fd, "w"));
    }

    // send a notification of the new state to the client
    fputs(tu_state_names[tu->state], write);
    return 0;
    // return -1 ???
}

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU was in the TU_CONNECTED state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the peer TU (the one to which the call is currently
 *     connected) simultaneously transitions to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_RINGING state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the called TU (which is in the TU_RING_BACK state)
 *     simultaneously transitions to the TU_DIAL_TONE state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *   If the TU was in any other state, then there is no change of state.
 *
 * In all cases, a notification of the new state is sent to the network client
 * underlying this TU.  In addition, if the previous state was TU_CONNECTED,
 * TU_RING_BACK, or TU_RINGING, then the peer, called, or calling TU is also
 * notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs.  Note that "error" refers to
 * an underlying I/O or other implementation error; a transition to the TU_ERROR
 * state (with no underlying implementation error) is considered a normal occurrence
 * and would result in 0 being returned.
 */
int tu_hangup(TU *tu) {
    if(tu->state == TU_CONNECTED) {
        TU* connection = tu->connection;
        tu->state = TU_ON_HOOK;
        connection->state = TU_DIAL_TONE;
        // change connection fields to NULL
        tu->connection=NULL;
        connection->connection=NULL;
    } else if(tu->state == TU_RING_BACK) {
        TU* connection = tu->connection;
        tu->state = TU_ON_HOOK;
        connection->state = TU_ON_HOOK;
        // change connection fields to NULL
        tu->connection=NULL;
        connection->connection=NULL;
    } else if(tu->state == TU_RINGING) {
        TU* connection = tu->connection;
        tu->state = TU_ON_HOOK;
        connection->state = TU_DIAL_TONE;
        // change connection fields to NULL
        tu->connection=NULL;
        connection->connection=NULL;
    } else if(tu->state == TU_DIAL_TONE || tu->state == TU_BUSY_SIGNAL || tu->state == TU_ERROR) {
        tu->state = TU_ON_HOOK;
    }

    // notification
    return 0;
}

/*
 * Dial an extension on a TU.
 *
 *   If the specified extension number does not refer to any currently registered
 *     extension, then the TU transitions to the TU_ERROR state.
 *   Otherwise, if the TU was in the TU_DIAL_TONE state, then what happens depends
 *     on the current state of the dialed extension:
 *       If the dialed extension was in the TU_ON_HOOK state, then the calling TU
 *         transitions to the TU_RING_BACK state and the dialed TU simultaneously
 *         transitions to the TU_RINGING state.
 *       If the dialed extension was not in the TU_ON_HOOK state, then the calling
 *         TU transitions to the TU_BUSY_SIGNAL state and there is no change to the
 *         state of the dialed extension.
 *   If the TU was in any state other than TU_DIAL_TONE, then there is no state change.
 *
 * In all cases, a notification of the new state is sent to the network client
 * underlying this TU.  In addition, if the new state is TU_RING_BACK, then the
 * called extension is also notified of its new state (i.e. TU_RINGING).
 *
 * @param tu  The tu on which the dialing operation is to be performed.
 * @param ext  The extension to be dialed.
 * @return 0 if successful, -1 if any error occurs.  Note that "error" refers to
 * an underlying I/O or other implementation error; a transition to the TU_ERROR
 * state (with no underlying implementation error) is considered a normal occurrence
 * and would result in 0 being returned.
 */
int tu_dial(TU *tu, int ext) {
    TU** clients = pbx->registry;
    TU* dialed = clients[ext];
    if(dialed==NULL) {
        tu->state = TU_ERROR;
    }
    if(tu->state == TU_DIAL_TONE) {
        if(dialed->state==TU_ON_HOOK) {
            tu->state = TU_RING_BACK;
            dialed->state = TU_RINGING;
            tu->connection = dialed;
            dialed->connection = tu;
        } else {
            tu->state = TU_BUSY_SIGNAL;
        }
    }
    // In all cases, a notification of the new state is sent
    // to the network client underlying this TU.
    int fd_tu = tu->fd;
    FILE* wt_tu = fdopen(fd_tu, "w");
    fputs(tu_state_names[tu->state], wt_tu);

    // In addition, if the new state is TU_RING_BACK,
    // then the called extension is also notified of its new state (i.e. TU_RINGING).
    if(tu->state == TU_RING_BACK) {
        int fd_dialed = dialed->fd;
        FILE* wt_dialed = fdopen(fd_dialed, "w");
        fputs(tu_state_names[dialed->state], wt_dialed);
    }

    return 0;
    // return -1 ???
}

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 */
int tu_chat(TU *tu, char *msg) {
    if(tu->state!=TU_CONNECTED) {
        return -1;
    } else {
        TU* connection = tu->connection;
        int fd = connection->fd;
        FILE* wt = fdopen(fd, "w");
        fputs(msg, wt);
        return 0;
    }
}