const std = @import("std");
const builtin = @import("builtin");


pub fn build(b: *std.Build) !void {
    const target = b.standardTargetOptions(.{});
    const optimize = b.standardOptimizeOption(.{});

    const raylib_dep = b.dependency("raylib", .{
        .target = target,
        .optimize = optimize,
    });
    const raylib_art = raylib_dep.artifact("raylib");

    const exe = b.addExecutable(.{
        .name = "fred",
        .target = target,
        .optimize = optimize,
    });

    // find all our source files
    var cppFiles = std.ArrayList([]const u8).init(b.allocator);
    {
        var dir = try std.fs.cwd().openDir("src", .{ .iterate = true });

        var walker = try dir.walk(b.allocator);
        defer walker.deinit();

        const allowed_exts = [_][]const u8{ ".cpp", ".cc" };
        while (try walker.next()) |entry| {
            const ext = std.fs.path.extension(entry.basename);
            const include_file = for (allowed_exts) |e| {
                if (std.mem.eql(u8, ext, e))
                    break true;
            } else false;
            if (include_file) {
                // we have to add the top level folder name ("src/") to make this a path relative to build.zig
                const relPath = try std.fs.path.join(b.allocator, &[_][]const u8{ "src", entry.path });
                try cppFiles.append(relPath);
            }
        }
    }

    const cppFlags = &[_][]const u8{ "-std=c++23", "-Wall", "-Werror" };
    exe.addCSourceFiles(.{ .files = cppFiles.items, .flags = cppFlags });
    exe.linkLibC();
    exe.linkLibCpp();
    exe.linkLibrary(raylib_art);


    const install_cmd = b.addInstallArtifact(exe, .{});
    b.getInstallStep().dependOn(&install_cmd.step);

    const run_cmd = b.addRunArtifact(exe);
    run_cmd.step.dependOn(&install_cmd.step);

    const run_step = b.step("run", "Run the app");
    run_step.dependOn(&run_cmd.step);
}
