-- Prince of Persia CI boot validation plugin for MAME
--
-- Monitors the 6502 PC each video frame for three ordered milestones derived
-- from `xasm++ --symbols` on MASTER.S:
--
--   FIRSTBOOT   = $F8A8  initial game boot entry point
--   ATTRACTMODE = $FBC8  attract loop / title animation
--   TITLESCREEN = $FC41  title screen display
--
-- Writes a result file at POP_CI_RESULT (env) or /tmp/pop_ci_result.txt, then
-- takes a screenshot via the MAME video API and exits.
--
-- MAME invocation (macOS self-hosted runner):
--   mame apple2e \
--     -flop1 PrinceOfPersia_SideA.nib \
--     -plugin pop_boot_check \
--     -pluginspath <repo>/tests/fixtures/mame/plugins \
--     -video bgfx -bgfx_backend metal \
--     -window -nomaximize \
--     -snappath /tmp/pop_mame_snap \
--     -skip_gameinfo -nomouse
--   (POP_CI_RESULT=/tmp/pop_ci_result.txt must be set in the environment)

local exports = {}
exports.name        = "pop_boot_check"
exports.version     = "1.0.0"
exports.description = "Prince of Persia CI boot validation"
exports.license     = "MIT"
exports.author      = "xasm++"
exports.type        = "general"

function exports.startplugin()
    -- Milestone addresses from xasm++ --symbols on MASTER.S
    local FIRSTBOOT   = 0xF8A8
    local ATTRACTMODE = 0xFBC8
    local TITLESCREEN = 0xFC41

    -- Detection window: ±256 bytes around each milestone address.
    -- The 6502 runs thousands of instructions between frame callbacks so an
    -- exact PC match would be missed; the window catches a single subroutine.
    local WINDOW = 0x100

    local milestone = { firstboot = false, attract = false, title = false }
    local frame_count  = 0
    local MAX_FRAMES   = 60 * 60 * 3   -- 3 minutes at ~60 fps
    local done         = false
    local exit_delay   = 0

    local result_path = os.getenv("POP_CI_RESULT") or "/tmp/pop_ci_result.txt"

    local function write_result(status, reason)
        local f = io.open(result_path, "w")
        if not f then
            print("[POP_CI] ERROR: cannot open result file: " .. result_path)
            return
        end
        f:write(status .. "\n")
        if reason then f:write("reason=" .. reason .. "\n") end
        f:write("firstboot=" .. tostring(milestone.firstboot) .. "\n")
        f:write("attract="   .. tostring(milestone.attract)   .. "\n")
        f:write("title="     .. tostring(milestone.title)     .. "\n")
        f:write("frames="    .. tostring(frame_count)          .. "\n")
        f:close()
        print("[POP_CI] Result written to " .. result_path .. ": " .. status)
    end

    emu.register_frame(function()
        if done then
            -- Give MAME a few frames to flush the snapshot before exiting
            exit_delay = exit_delay + 1
            if exit_delay >= 15 then
                manager.machine:exit()
            end
            return
        end

        frame_count = frame_count + 1

        -- Safely fetch the main CPU; the device tree may not be ready on frame 1
        local ok_cpu, cpu = pcall(function()
            return manager.machine.devices[":maincpu"]
        end)
        if not ok_cpu or not cpu then return end

        local ok_pc, pc = pcall(function()
            return cpu.state["PC"].value
        end)
        if not ok_pc then return end

        -- Milestone detection (ordered — later milestones only checked once earlier ones hit)
        if not milestone.firstboot
                and pc >= FIRSTBOOT and pc < FIRSTBOOT + WINDOW then
            milestone.firstboot = true
            print(string.format("[POP_CI] FIRSTBOOT   PC=$%04X frame=%d", pc, frame_count))
        end

        if milestone.firstboot and not milestone.attract
                and pc >= ATTRACTMODE and pc < ATTRACTMODE + WINDOW then
            milestone.attract = true
            print(string.format("[POP_CI] ATTRACTMODE PC=$%04X frame=%d", pc, frame_count))
        end

        if milestone.attract and not milestone.title
                and pc >= TITLESCREEN and pc < TITLESCREEN + WINDOW then
            milestone.title = true
            print(string.format("[POP_CI] TITLESCREEN PC=$%04X frame=%d", pc, frame_count))

            -- Attempt screenshot; requires -video bgfx or similar (not -video none)
            pcall(function() manager.machine.video:save_snapshot() end)

            write_result("PASS", nil)
            done = true
            return
        end

        -- Timeout guard
        if frame_count >= MAX_FRAMES then
            print(string.format("[POP_CI] TIMEOUT after %d frames (%.1f s)",
                frame_count, frame_count / 60.0))
            write_result("FAIL", "timeout")
            done = true
        end
    end)
end

return exports
